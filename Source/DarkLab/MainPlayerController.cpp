// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"
#include "MainCharacter.h"
#include "GameFramework/GameModeBase.h"

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

// Show/Hide menu
void AMainPlayerController::ShowHideMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowHideMenu called"));

	// TODO
}

// Takes one 'life' and calls CalculateLoss
void AMainPlayerController::OnDisabled()
{
	if (Lives > 0)
	{
		--Lives;
		UE_LOG(LogTemp, Warning, TEXT("Disabled once"));

		UnPossess();
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
		// TODO Use Delay to let the character die
		// TODO make gamemode do this
		Enable();
	}
}
// Called on loss
void AMainPlayerController::OnLoss()
{
	UE_LOG(LogTemp, Warning, TEXT("You actually lost!"));

	// TODO
}
// Respawns the character
void AMainPlayerController::Enable()
{
	GetWorld()->GetAuthGameMode()->RestartPlayer(this);
	MainCharacter = Cast<AMainCharacter>(GetCharacter());
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

	InputComponent->BindAction("Menu/Cancel", IE_Pressed, this, &AMainPlayerController::ShowHideMenu);
}

// Called every frame
void AMainPlayerController::PlayerTick(const float deltaTime)
{
	Super::PlayerTick(deltaTime);

	if (bLookWithMouse)
		LookWithMouse();

	// We try to look with stick anyway
	// If we get input, we start looking with stick
	LookWithStick();
}