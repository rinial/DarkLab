// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DarknessController.generated.h"

// Controls the darkness
UCLASS()
class DARKLAB_API ADarknessController : public AAIController
{
	GENERATED_BODY()

private:
	// Player character reference
	class AMainCharacter* Character;
	// Controlled darkness
	class ADarkness* Darkness;
	
public:
	// Starts following the player
	void TrackPlayer();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};