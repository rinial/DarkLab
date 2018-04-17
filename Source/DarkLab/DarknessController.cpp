// Fill out your copyright notice in the Description page of Project Settings.

#include "DarknessController.h"

// Starts following the player
void ADarknessController::TrackPlayer()
{
	MoveToLocation(FVector(0, 0, 0), 1, true, false);
	UE_LOG(LogTemp, Warning, TEXT("Going somewhere"));

	if (IsFollowingAPath())
	{
		UE_LOG(LogTemp, Warning, TEXT("Following"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not following"));
	}
	
	/*
	APlayerController* Controller = GetWorld()->GetFirstPlayerController();

	// We make the darkness follow the player untill it reaches him
	ACharacter* Character = Controller->GetCharacter();
	if (Character)
	{
		MoveToActor((AActor*)Character, 0, true, true);
		UE_LOG(LogTemp, Warning, TEXT("Following the player"));
	}
	*/
}

// Called when the game starts or when spawned
void ADarknessController::BeginPlay()
{
	if (IsFollowingAPath())
	{
		UE_LOG(LogTemp, Warning, TEXT("Following"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not following"));
	}

	// We initialize the tracking
	TrackPlayer();

	// TODO delete later
	FTimerHandle handler;
	((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &ADarknessController::TrackPlayer, 1.0f, true, 0.0f);
}