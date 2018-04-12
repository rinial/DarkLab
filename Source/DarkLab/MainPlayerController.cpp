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

	// TODO
}

void AMainPlayerController::SetupInputComponent()
{
	// TODO
}

void AMainPlayerController::LookWithStick()
{
	// TODO
}

void AMainPlayerController::LookWithMouse()
{
	// TODO
}