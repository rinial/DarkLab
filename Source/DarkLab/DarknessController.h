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

private:
	// Controlled darkness
	UPROPERTY()
	class ADarkness* Darkness;

public:
	// If true, keeps hunting till it gets the player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Hunting")
	bool bIsPersistent = false;

protected:
	// The maximum time the darkness keeps being passive
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Passive")
	float MaxTimePassive = 35.0f;
	// The minimum time the darkness keeps veing passive
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Passive")
	float MinTimePassive = 15.0f;
	// Currently decided max passive time updated at the start of every passive phase
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Passive")
	float CurrentMaxTimePassive = 25.0f;

	//// The time between a character 'death' and restart of tracking
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Hunting")
	//float TrackingRestartDelay = 8.0f;

	// The minimum distance between darkness and character before darkness can teleport
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Teleport")
	float MinTeleportDistance = 2500.0f;
	// The minimum time between two consequtive teleports
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Teleport")
	float MinTimeBetweenTeleports = 15.0f;
	// Time since last teleport
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Teleport")
	float SinceLastTeleport = 0.f;

	// The maximum time the darkness keeps hunting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Hunting")
	float MaxTimeHunting = 60.0f;
	// The minimum time the darkness keeps hunting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Hunting")
	float MinTimeHunting = 30.0f;
	// Currently decided max hunting time updated at the start of every hunt
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Hunting")
	float CurrentMaxTimeHunting = 45.0f;

	// The maximum time for any retreating 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Retreating")
	float MaxTimeRetreating = 25.f;

	// The minimum time spend in the dark before it can teleport or just stop retreating
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: States")
	float MinTimeInDark = 5.0f;

	// Time since last state change
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: States")
	float SinceLastStateChange = 0.f;

public:
	// Current state of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: States")
	EDarkStateEnum State = EDarkStateEnum::VE_Passive;

public:
	// Called on disabling a character
	void OnDisabling();

	// Called when player gets the black doorcard
	void OnPlayerFindsBlackCard();

	// Teleports to some point closer to character
	UFUNCTION(BlueprintCallable, Category = "Darkness: Teleport")
	void TeleportToCharacter();
	
	// Stops everything, enters passive state
	UFUNCTION(BlueprintCallable, Category = "Darkness: Passive")
	void BecomePassive();
	// Starts following the player
	UFUNCTION(BlueprintCallable, Category = "Darkness: Hunting")
	void StartHunting();
	// Stops following the player and retreats into the darkness
	UFUNCTION(BlueprintCallable, Category = "Darkness: Retreating")
	void StartRetreating();

public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(const float deltaTime) override;
};