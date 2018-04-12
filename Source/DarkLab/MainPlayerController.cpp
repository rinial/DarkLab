// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"
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

	// TODO update look direction
}

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// TODO delete later
	// For future use
	/*InputComponent->BindAction("SetDestination", IE_Pressed, this, &ADarkLabPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &ADarkLabPlayerController::OnSetDestinationReleased);*/

	check(InputComponent);

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

void AMainPlayerController::LookWithStick()
{
	// TODO
}

void AMainPlayerController::LookWithMouse()
{
	// TODO delete later
	// For future use
	/*FHitResult TraceHitResult;
	PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
	FVector CursorFV = TraceHitResult.ImpactNormal;
	FRotator CursorR = CursorFV.Rotation();
	CursorToWorld->SetWorldLocation(TraceHitResult.Location);
	CursorToWorld->SetWorldRotation(CursorR);
	
	// Trace to see what is under the mouse cursor
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(Hit.ImpactPoint);
	}*/

	// TODO
}