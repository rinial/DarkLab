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
	// Controlled darkness
	UPROPERTY()
	class ADarkness* Darkness;

protected:
	// The time between a character 'death' and restart of tracking
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness Controller: Tracking")
	float TrackingRestartDelay = 8.0f;

public:
	// Called on disabling a character
	void OnDisabling();
	
public:
	// Starts following the player
	UFUNCTION(BlueprintCallable, Category = "Darkness Controller: Tracking")
	void TrackPlayer();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};