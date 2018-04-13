// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"

// Movement controls
void AMainPlayerController::MoveUp(const float value)
{
	if (!Character)
		return;
	
	Character->MoveUp(value);
}
void AMainPlayerController::MoveRight(const float value)
{
	if (!Character)
		return;
	
	Character->MoveRight(value);
}

// Rotation controls
void AMainPlayerController::LookWithMouse()
{
	if (!Character)
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
	if (!Character)
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
		bShowMouseCursor = true;
	}

	Character->Look(direction);
}

// Sets default values
AMainPlayerController::AMainPlayerController()
{
	// Show cursor in game
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::CardinalCross;

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

	//check(InputComponent);

	// TODO delete later
	// For future use
	/*InputComponent->BindAction("SetDestination", IE_Pressed, this, &AMainPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AMainPlayerController::OnSetDestinationReleased);*/

	InputComponent->BindAxis("MoveUp", this, &AMainPlayerController::MoveUp);
	InputComponent->BindAxis("MoveRight", this, &AMainPlayerController::MoveRight);

	InputComponent->BindAxis("LookUp");
	InputComponent->BindAxis("LookRight");
}

// Called every frame
void AMainPlayerController::PlayerTick(const float deltaTime)
{
	Super::PlayerTick(deltaTime);

	if(bLookWithMouse)
		LookWithMouse();

	// We try to look with stick anyway
	// If we get input, we start looking with stick
	LookWithStick();
}