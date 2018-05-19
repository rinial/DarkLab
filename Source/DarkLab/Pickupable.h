// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Pickupable.generated.h"

class AMainCharacter;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPickupable : public UInterface
{
	GENERATED_BODY()
};

// Represents objects that can be picked up by the main character
class DARKLAB_API IPickupable
{
	GENERATED_BODY()

public:
	// Called when the object is to be picked up
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickupable")
	void PickUp(AMainCharacter* character);
};