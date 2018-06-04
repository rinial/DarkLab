// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicEquipableObject.h"
#include "Usable.h"
#include "Lighter.generated.h"

// A simple lighter
UCLASS(Blueprintable)
class DARKLAB_API ALighter : public ABasicEquipableObject, public IUsable
{
	GENERATED_BODY()
	
public:
	// Enables or disables light
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Usable")
	void Use();
	virtual void Use_Implementation() override;

	// TODO let some interface define it?
	// Resets to initial state
	UFUNCTION(BlueprintCallable, Category = "Lighter")
	void Reset();

	// Resets only power level
	UFUNCTION(BlueprintCallable, Category = "Lighter")
	void ResetPowerLevel();

	// Returns true if lighter is on
	UFUNCTION(BlueprintCallable, Category = "Lighter")
	bool IsOn();

	// Called when turned on
	UFUNCTION(BlueprintImplementableEvent, Category = "Lighter")
	void OnTurnOn();
	// Called when turned off
	UFUNCTION(BlueprintImplementableEvent, Category = "Lighter")
	void OnTurnOff();
	// Called when trying to turn on but failing
	UFUNCTION(BlueprintImplementableEvent, Category = "Lighter")
	void OnTurnOnFailed();

protected:
	// Lighter's direction and root object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighter: Components")
	class UArrowComponent* Direction;

	// Main light
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighter: Components")
	class UPointLightComponent* Light;

public:
	// Current power level from 0 to 1
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighter")
	float PowerLevel = 1.f;

	// The speed of losing power
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighter")
	float PowerLossPerSecond = 0.002f;

private:
	FLinearColor NormalColor = FLinearColor::White; // TODO change
	float CurrentlySeenLevel = 1.f;

public:
	// Sets default values
	ALighter();

protected:
	// Called every frame
	virtual void Tick(const float deltaTime) override;
};