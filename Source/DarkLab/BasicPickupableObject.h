// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicActivatableObject.h"
#include "Pickupable.h"
#include "BasicPickupableObject.generated.h"

// Represents objects with physical representation that can be picked up by the main character
UCLASS()
class DARKLAB_API ABasicPickupableObject : public ABasicActivatableObject, public IPickupable
{
	GENERATED_BODY()
	
public:
	// Called when the object is to be picked up
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickupable")
	void PickUp(AMainCharacter* character);
	virtual void PickUp_Implementation(AMainCharacter* character) override;
	
	// Called when the object is activated (picked up in the laboratory)
	UFUNCTION(BlueprintCallable, Category = "Activatable")
	virtual void ActivateObject(AMainCharacter* character) override;

public:
	// Sets default values
	ABasicPickupableObject();
};