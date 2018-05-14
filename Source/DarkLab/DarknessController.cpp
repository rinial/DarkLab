// Fill out your copyright notice in the Description page of Project Settings.

#include "DarknessController.h"
#include "Darkness.h"
#include "GameFramework/PawnMovementComponent.h"
// For on screen debug
#include "EngineGlobals.h"
#include "Engine/Engine.h"

// Shows/hides debug
void ADarknessController::SetShowDebug(bool show)
{
	Darkness->bShowLightDebug = show;
}

// Called on disabling a character
void ADarknessController::OnDisabling()
{
	State = EDarkStateEnum::VE_Retreating;
	Darkness->Stop();

	// Start tracking a new one after a delay
	FTimerHandle handler;
	((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &ADarknessController::TrackPlayer, 1.0f, false, TrackingRestartDelay);

	// TODO delete later
	// Can be used somewhere
	/*FTimerHandle handler;
	((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &ADarknessController::TrackPlayer, 5.0f, true, 0.0f);*/
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
			State = EDarkStateEnum::VE_Hunting;
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
	if (!Darkness)
		UE_LOG(LogTemp, Warning, TEXT("NoDarkness"));

	// Then we initiate the tracking
	TrackPlayer();
}

// Called every frame
void ADarknessController::Tick(const float deltaTime)
{
	Super::Tick(deltaTime);

	// Darkness retreats from powerful light sources
	bool isRetreating = Darkness->RetreatFromLight();

	// Tracks if isn't retreating already
	if (!isRetreating)
	{
		switch (State)
		{
		case EDarkStateEnum::VE_Passive:
			break;
		case EDarkStateEnum::VE_Hunting:
			Darkness->Tracking();
			break;
		case EDarkStateEnum::VE_Retreating:
			Darkness->IntoDarkness();
			break;
		}
	}

	// TODO delete later: used for debug
	if (GEngine)
	{
		FString state = "Unknown";
		switch (State)
		{
		case EDarkStateEnum::VE_Passive:
			state = "Passive";
			break;
		case EDarkStateEnum::VE_Hunting:
			state = "Hunting";
			break;
		case EDarkStateEnum::VE_Retreating:
			state = "Retreating";
			break;
		}
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Darkness state: %s"), *state), true);

		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT(""), true);
	}
}