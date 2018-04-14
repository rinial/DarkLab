// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Usable.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UUsable : public UInterface
{
	GENERATED_BODY()
};

// Represents objects that can be used (not on scene)
class DARKLAB_API IUsable
{
	GENERATED_BODY()

public:
	// Called when the object is used (usually equiped or directly from inventory)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Usable")
	void Use();
};