// Fill out your copyright notice in the Description page of Project Settings.

#include "DarknessController.h"
#include "MainCharacter.h"
#include "Darkness.h"
#include "Runtime/Engine/Classes/GameFramework/PawnMovementComponent.h"

// Starts following the player
void ADarknessController::TrackPlayer()
{	
	Darkness->MoveToActor((AActor*)Character);
	UE_LOG(LogTemp, Warning, TEXT("Following the player"));
}

// Called when the game starts or when spawned
void ADarknessController::BeginPlay()
{
	Super::BeginPlay();

	// We first find the darkness
	Darkness = Cast<ADarkness>(GetPawn());

	// Then we find the character
	APlayerController* controller = GetWorld()->GetFirstPlayerController();
	if (controller)
	{
		ACharacter* character = controller->GetCharacter();
		if (character)
			Character = Cast<AMainCharacter>(character);
	}

	// We initiate the tracking
	TrackPlayer();

	// TODO delete
	// saved for later usage
	/*FTimerHandle handler;
	((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &ADarknessController::TrackPlayer, 1.0f, true, 0.0f);*/
}