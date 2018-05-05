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
	// Sets default values
	AFlashlight();
};