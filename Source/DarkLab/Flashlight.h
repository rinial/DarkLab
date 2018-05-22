// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicEquipableObject.h"
#include "Usable.h"
#include "Flashlight.generated.h"

// A simple flashlight
UCLASS(Blueprintable)
class DARKLAB_API AFlashlight : public ABasicEquipableObject, public IUsable
{
	GENERATED_BODY()
	
public:
	// Enables or disables light
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Usable")
	void Use();
	virtual void Use_Implementation() override;

	// TODO let some interface define it?
	// Resets to initial state
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void Reset();

	// Resets only power level
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void ResetPowerLevel();

	// Returns true if flashlight is on
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	bool IsOn();

protected:
	// Flashlight's direction and root object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight: Components")
	class UArrowComponent* Direction;

	// Main light
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight: Components")
	class USpotLightComponent* MainLight;

	// Extra light
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight: Components")
	class UPointLightComponent* ExtraLight;

public:
	// Current power level from 0 to 1
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
	float PowerLevel = 1.f;

	// The speed of losing power
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight")
	float PowerLossPerSecond = 0.03f;

private:
	FLinearColor NormalColor = FLinearColor::White;

public:
	// Sets default values
	AFlashlight();

protected:
	// Called every frame
	virtual void Tick(const float deltaTime) override;
};