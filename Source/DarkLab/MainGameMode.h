// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.generated.h"

// Controls the game
UCLASS(Blueprintable)
class DARKLAB_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Returns the light level and the location of the brightest light
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const bool sixPoints = false, const float sixPointsRadius = 30.0f);
	float GetLightingAmount(FVector& lightLoc, const FVector location, const bool sixPoints = false, const float sixPointsRadius = 30.0f);
	float GetLightingAmount(FVector& lightLoc, const TArray<FVector> locations);
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const FVector location, const bool sixPoints = false, const float sixPointsRadius = 30.0f);
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const TArray<FVector> locations);
	
public:
	// Sets default values
	AMainGameMode();
};