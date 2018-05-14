// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DarknessController.generated.h"

// Darkness states
UENUM(BlueprintType)
enum class EDarkStateEnum : uint8
{
	VE_Passive 	UMETA(DisplayName = "Passive"),
	VE_Hunting 	UMETA(DisplayName = "Hunting"),
	VE_Retreating	UMETA(DisplayName = "Retreating")
};

// Controls the darkness
UCLASS()
class DARKLAB_API ADarknessController : public AAIController
{
	GENERATED_BODY()

public:
	// Shows/hides debug
	UFUNCTION()
	void SetShowDebug(bool show);

private:
	// Controlled darkness
	UPROPERTY()
	class ADarkness* Darkness;

protected:
	// The time between a character 'death' and restart of tracking
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness Controller: Tracking")
	float TrackingRestartDelay = 8.0f;

public:
	// Current state of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: State")
	EDarkStateEnum State = EDarkStateEnum::VE_Passive;

public:
	// Called on disabling a character
	void OnDisabling();
	
public:
	// Starts following the player
	UFUNCTION(BlueprintCallable, Category = "Darkness Controller: Tracking")
	void TrackPlayer();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(const float deltaTime) override;
};