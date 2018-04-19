// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"
#include "MainCharacter.h"
#include "Runtime/Engine/Classes/GameFramework/GameModeBase.h"

// Movement controls
void AMainPlayerController::MoveUp(const float value)
{
	if (Character && bCharacterActive)
		Character->MoveUp(value);
}
void AMainPlayerController::MoveRight(const float value)
{
	if (Character && bCharacterActive)
		Character->MoveRight(value);
}

// Rotation controls
void AMainPlayerController::LookWithMouse()
{
	if (!(Character && bCharacterActive))
		return;

	FVector mLocation, mDirection;
	if (!DeprojectMousePositionToWorld(mLocation, mDirection))
		return;

	const FVector cLocation = Character->GetActorLocation();

	FVector direction = mLocation - cLocation;
	// TODO change something to use camera lag
	//FVector direction = mLocation + mDirection * Character->CameraBoom->CameraLagSpeed - cLocation;
	//FVector direction = mLocation - Character->TopDownCamera->GetComponentLocation();
	direction.Z = 0.0f;

	Character->Look(direction);
}
void AMainPlayerController::LookWithStick()
{
	if (!(Character && bCharacterActive))
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

	Character->Look(direction);
}

// Makes the character use something he has equiped
void AMainPlayerController::UseEquiped()
{
	if (Character && bCharacterActive)
		Character->UseEquiped();
}
// Makes the character avtivate smth near him
void AMainPlayerController::Activate()
{
	if (Character && bCharacterActive)
		Character->Activate();
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

		bCharacterActive = false;

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
	Character = Cast<AMainCharacter>(GetCharacter());
	bCharacterActive = true;
}

// Sets default values
AMainPlayerController::AMainPlayerController()
{
	// Show cursor in game
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;

	// Can't get character at this point
	Character = nullptr;
}

// Called when the game starts or when spawned
void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<AMainCharacter>(GetCharacter());
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