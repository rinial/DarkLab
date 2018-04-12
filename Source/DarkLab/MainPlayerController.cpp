// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"
#include "DrawDebugHelpers.h"
#include "MainCharacter.h"

AMainPlayerController::AMainPlayerController()
{
	// Show cursor in game
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AMainPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if(bLookWithMouse)
		LookWithMouse();
	// We try to look with stick anyway
	// If we get input, we start looking with stick
	LookWithStick();
}

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);

	// TODO delete later
	// For future use
	/*InputComponent->BindAction("SetDestination", IE_Pressed, this, &ADarkLabPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &ADarkLabPlayerController::OnSetDestinationReleased);*/

	InputComponent->BindAxis("MoveUp", this, &AMainPlayerController::MoveUp);
	InputComponent->BindAxis("MoveRight", this, &AMainPlayerController::MoveRight);

	InputComponent->BindAxis("LookUp");
	InputComponent->BindAxis("LookRight");
}

void AMainPlayerController::MoveUp(float Value)
{
	AMainCharacter* Character = Cast<AMainCharacter>(GetCharacter());
	if (Character)
		Character->MoveUp(Value);
}

void AMainPlayerController::MoveRight(float Value)
{
	AMainCharacter* Character = Cast<AMainCharacter>(GetCharacter());
	if (Character)
		Character->MoveRight(Value);
}

void AMainPlayerController::LookWithMouse()
{
	AMainCharacter* Character = Cast<AMainCharacter>(GetCharacter());
	if (Character)
	{
		FVector mLocation , mDirection;
		DeprojectMousePositionToWorld(mLocation, mDirection);

		FVector cLocation = Character->GetActorLocation();

		FVector direction = mLocation - cLocation;
		// TODO change something to use camera lag
		//FVector direction = mLocation + mDirection * Character->CameraBoom->CameraLagSpeed - cLocation;
		//FVector direction = mLocation - Character->TopDownCamera->GetComponentLocation();
		direction.Z = 0.0f; 

		Character->Look(direction);
	}
}

void AMainPlayerController::LookWithStick()
{
	AMainCharacter* Character = Cast<AMainCharacter>(GetCharacter());
	if (Character)
	{
		float XValue = this->InputComponent->GetAxisValue("LookUp");
		float YValue = this->InputComponent->GetAxisValue("LookRight");

		FVector direction(XValue, YValue, 0.0f);

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
}