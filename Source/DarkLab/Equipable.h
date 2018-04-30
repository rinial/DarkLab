// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Equipable.generated.h"

class AMainCharacter;

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UEquipable : public UInterface
{
	GENERATED_BODY()
};

// Represents objects that can be equiped by the main character
class DARKLAB_API IEquipable
{
	GENERATED_BODY()

public:
	// Called when the object is to be equiped
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipable")
	void Equip(AMainCharacter* character, FName location);

	// Called when the object is to be unequiped
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipable")
	void Unequip(AMainCharacter* character);
};
