// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"
#include "MainCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.h"
#include "Lighter.h"

// Movement controls
void AMainPlayerController::MoveUp(const float value)
{
	if (MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->MoveUp(value);
}
void AMainPlayerController::MoveRight(const float value)
{
	if (MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->MoveRight(value);
}

// Rotation controls
void AMainPlayerController::LookWithMouse()
{
	if (!MainCharacter || MainCharacter->bIsDisabled)
		return;

	FVector mLocation, mDirection;
	if (!DeprojectMousePositionToWorld(mLocation, mDirection))
		return;

	const FVector cLocation = MainCharacter->GetActorLocation();

	FVector direction = mLocation - cLocation;
	// TODO change something to use camera lag
	//FVector direction = mLocation + mDirection * MainCharacter->CameraBoom->CameraLagSpeed - cLocation;
	//FVector direction = mLocation - MainCharacter->TopDownCamera->GetComponentLocation();
	direction.Z = 0.0f;

	MainCharacter->Look(direction);
}
void AMainPlayerController::LookWithStick()
{
	if (!MainCharacter || MainCharacter->bIsDisabled)
		return;

	const float xValue = InputComponent->GetAxisValue("LookUp");
	const float yValue = InputComponent->GetAxisValue("LookRight");

	FVector direction(xValue, yValue, 0.0f);

	// Doesn't work on small direction vectors to prevent undesired rotations
	// Also doesn't override bLookWithMouse if no input is found
	if (direction.Size() < 0.25f)
		return;

	if (bLookWithMouse)
	{
		// Now we look with stick
		bLookWithMouse = false;
		// And we dont need cursor
		bShowMouseCursor = false;
	}

	MainCharacter->Look(direction);
}

// Makes the character use something he has equiped
void AMainPlayerController::UseEquiped()
{
	if (MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->UseEquiped();
}
// Makes the character avtivate smth near him
void AMainPlayerController::Activate()
{
	if (MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->Activate();
}
// Equipes first item
void AMainPlayerController::Equip1()
{
	if (MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->Equip1();
}
// Equipes second item
void AMainPlayerController::Equip2()
{
	if (MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->Equip2();
}

// Show/Hide menu
void AMainPlayerController::ShowHideMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowHideMenu called"));

	// TODO delete
	// Exits
	FGenericPlatformMisc::RequestExit(false);

	// TODO
}

// Resets map, only used for debug
void AMainPlayerController::ResetMap()
{
	// UE_LOG(LogTemp, Warning, TEXT("ResetMap called"));

	Cast<AMainGameMode>(GetWorld()->GetAuthGameMode())->ResetMap();
}

// Takes one 'life' and calls CalculateLoss
void AMainPlayerController::OnDisabled()
{
	if (Lives > 0)
	{
		--Lives;
		UE_LOG(LogTemp, Warning, TEXT("Disabled once"));

		CalculateLoss();
	}
}
// Checks for the loss and calls OnLoss
void AMainPlayerController::CalculateLoss()
{
	if (Lives <= 0)
		OnLoss();
	else
	{
		// TODO make gamemode do this
		// Respawn after a delay
		FTimerHandle handler;
		((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &AMainPlayerController::Enable, 1.0f, false, RespawnDelay);
	}
}
// Called on loss
void AMainPlayerController::OnLoss()
{
	Cast<AMainGameMode>(GetWorld()->GetAuthGameMode())->OnLoss();

	// TODO
}
// Respawns the character
void AMainPlayerController::Enable()
{
	// We only unposses at this point, otherwise we would have no character during the respawn delay
	// UnPossess();
	// GetWorld()->GetAuthGameMode()->RestartPlayer(this);
	// MainCharacter = Cast<AMainCharacter>(GetCharacter());

	MainCharacter->Enable();
}

// Sets default values
AMainPlayerController::AMainPlayerController()
{
	// Show cursor in game
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

// Called when the game starts or when spawned
void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	MainCharacter = Cast<AMainCharacter>(GetCharacter());

	GetMousePosition(LastMousePosition.X, LastMousePosition.Y);

	// We spawn a lighter for the character
	ALighter* lighter = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode())->SpawnLighter(0, 0);
	lighter->ActivateObject(MainCharacter);
	lighter->Execute_Use(lighter);
}

// Sets controls
void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// check(InputComponent);

	// Bind axis and buttons for use

	InputComponent->BindAxis("MoveUp", this, &AMainPlayerController::MoveUp);
	InputComponent->BindAxis("MoveRight", this, &AMainPlayerController::MoveRight);

	InputComponent->BindAxis("LookUp");
	InputComponent->BindAxis("LookRight");

	InputComponent->BindAction("UseEquiped", IE_Pressed, this, &AMainPlayerController::UseEquiped);
	InputComponent->BindAction("Activate", IE_Pressed, this, &AMainPlayerController::Activate);
	InputComponent->BindAction("Equip1", IE_Pressed, this, &AMainPlayerController::Equip1);
	InputComponent->BindAction("Equip2", IE_Pressed, this, &AMainPlayerController::Equip2);

	InputComponent->BindAction("Menu/Cancel", IE_Pressed, this, &AMainPlayerController::ShowHideMenu);

	InputComponent->BindAction("DebugReset", IE_Pressed, Cast<AMainGameMode>(GetWorld()->GetAuthGameMode()), &AMainGameMode::ResetMap);
	InputComponent->BindAction("DebugShow", IE_Pressed, Cast<AMainGameMode>(GetWorld()->GetAuthGameMode()), &AMainGameMode::ShowHideDebug);
}

// Called every frame
void AMainPlayerController::PlayerTick(const float deltaTime)
{
	Super::PlayerTick(deltaTime);

	// We update LastMousePosition and make sure to enable mouse controls if mouse was moved
	FVector2D newMousePosition;
	GetMousePosition(newMousePosition.X, newMousePosition.Y);
	if ((LastMousePosition - newMousePosition).Size() > 0.0f)
	{	// We look with mouse again
		bLookWithMouse = true;
		// And we need cursor
		bShowMouseCursor = true;
	}
	LastMousePosition = newMousePosition;

	if (bLookWithMouse)
		LookWithMouse();

	// We try to look with stick anyway
	// If we get input, we start looking with stick
	LookWithStick();
}