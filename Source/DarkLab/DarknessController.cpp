// Fill out your copyright notice in the Description page of Project Settings.

#include "DarknessController.h"
#include "Darkness.h"
#include "MainPlayerController.h"
#include "GameFramework/PawnMovementComponent.h"

// Called on disabling a character
void ADarknessController::OnDisabling()
{
	StartRetreating();

	//// Start tracking a new one after a delay
	//FTimerHandle handler;
	//((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &ADarknessController::StartHunting, 1.0f, false, TrackingRestartDelay);
}

// Called when player gets the black doorcard
void ADarknessController::OnPlayerFindsBlackCard()
{
	bIsPersistent = true;
	StartHunting();
}

// Stops everything, enters passive state
void ADarknessController::BecomePassive()
{
	State = EDarkStateEnum::VE_Passive;
	Darkness->Stop();

	CurrentMaxTimePassive = FMath::FRandRange(MinTimePassive, MaxTimePassive);
	SinceLastStateChange = 0.f;
	UE_LOG(LogTemp, Warning, TEXT("Entering passive state"));
}

// Teleports somewhere and starts following the player
void ADarknessController::StartHunting()
{	
	APlayerController* controller = GetWorld()->GetFirstPlayerController();
	if (!controller)
		return;

	AMainPlayerController* mainController = Cast<AMainPlayerController>(controller);
	if (!mainController)
		return;

	// Don't hunt anymore
	if (mainController->Lives <= 0)
		return;

	ACharacter* character = controller->GetCharacter();
	if (character)
	{
		State = EDarkStateEnum::VE_Hunting;
		CurrentMaxTimeHunting = FMath::FRandRange(MinTimeHunting, MaxTimeHunting);
		SinceLastStateChange = 0.f;
		Darkness->MoveToActor((AActor*)character);
		UE_LOG(LogTemp, Warning, TEXT("Starting the hunt"));
	}
}

// Stops following the player and retreats into the darkness
void ADarknessController::StartRetreating()
{
	State = EDarkStateEnum::VE_Retreating;
	Darkness->Stop();

	SinceLastStateChange = 0.f;
	UE_LOG(LogTemp, Warning, TEXT("Retreating into darkness"));
}

// Called when the game starts or when spawned
void ADarknessController::BeginPlay()
{
	Super::BeginPlay();

	// We find the darkness
	Darkness = Cast<ADarkness>(GetPawn());	
	if (!Darkness)
		UE_LOG(LogTemp, Warning, TEXT("No Darkness"));

	// Then we set the state
	BecomePassive();
}

// Called every frame
void ADarknessController::Tick(const float deltaTime)
{
	Super::Tick(deltaTime);

	SinceLastStateChange += deltaTime;

	// Darkness retreats from powerful light sources
	bool isAfraid = Darkness->RetreatFromLight();

	// Do something based on state if it isn't afraid already
	if (!isAfraid)
	{
		switch (State)
		{
		case EDarkStateEnum::VE_Passive:
			// If its been some time, start the hunt
			if (SinceLastStateChange >= CurrentMaxTimePassive)
				StartHunting();
			// TODO else?
			break;
		case EDarkStateEnum::VE_Hunting:
			// If hunting for some time, start retreating
			if (!bIsPersistent && SinceLastStateChange >= CurrentMaxTimeHunting)
				StartRetreating();
			// Otherwise keep hunting
			else
				Darkness->Tracking();
			break;
		case EDarkStateEnum::VE_Retreating:
			// If retreating for too long or if already escaped into darkness, become passive
			if (SinceLastStateChange >= MaxTimeRetreating || Darkness->TimeInDark >= MinTimeInDark)
				BecomePassive();
			// Otherwise keep retreating
			else
				Darkness->IntoDarkness();
			break;
		}
	}
}