// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Placeable.h"
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

protected:
	// Places an object on the map (size < 1 means that we take current size for this axis)
	// TODO return false if can't place?
	void PlaceObject(TScriptInterface<IPlaceable>& object, const int botLeftLocX, const int botLeftLocY, const EDirectionEnum direction = EDirectionEnum::VE_Up, const bool setSizeFirst = false, const int sizeX = 1, const int sizeY = 1, const int sizeZ = 0);
	void PlaceObject(TScriptInterface<IPlaceable>& object, const int botLeftLocX, const int botLeftLocY, const int botLeftLocZ, const EDirectionEnum direction = EDirectionEnum::VE_Up, const bool setSizeFirst = false, const int sizeX = 1, const int sizeY = 1, const int sizeZ = 0);
	void PlaceObject(TScriptInterface<IPlaceable>& object, const FIntVector botLeftLoc = FIntVector(), const EDirectionEnum direction = EDirectionEnum::VE_Up, const bool setSizeFirst = false, const int sizeX = 1, const int sizeY = 1, const int sizeZ = 0);
	
public:
	// Sets default values
	AMainGameMode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};