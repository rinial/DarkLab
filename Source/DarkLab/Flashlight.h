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

private:
	// Flashlight's direction and root object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* Direction;

	// Main light
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = "true"))
	class USpotLightComponent* MainLight;

	// Extra light
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = "true"))
	class UPointLightComponent* ExtraLight;

public:
	// Sets default values
	AFlashlight();
};
