// Fill out your copyright notice in the Description page of Project Settings.

#include "DarknessController.h"
#include "Darkness.h"
#include "GameFramework/PawnMovementComponent.h"

// Called on disabling a character
void ADarknessController::OnDisabling()
{
	Darkness->Stop();

	// Start tracking a new one after a delay
	FTimerHandle handler;
	((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &ADarknessController::TrackPlayer, 1.0f, false, TrackingRestartDelay);
}

// Starts following the player
void ADarknessController::TrackPlayer()
{	
	// Then we find the character
	APlayerController* controller = GetWorld()->GetFirstPlayerController();
	if (controller)
	{
		ACharacter* character = controller->GetCharacter();
		if (character)
		{
			Darkness->MoveToActor((AActor*)character);
			UE_LOG(LogTemp, Warning, TEXT("Following the player"));
		}
	}
}

// Called when the game starts or when spawned
void ADarknessController::BeginPlay()
{
	Super::BeginPlay();

	// We first find the darkness
	Darkness = Cast<ADarkness>(GetPawn());	

	// Then we initiate the tracking
	TrackPlayer();

	// TODO delete later
	// Can be used somewhere
	/*FTimerHandle handler;
	((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &ADarknessController::TrackPlayer, 5.0f, true, 0.0f);*/
}