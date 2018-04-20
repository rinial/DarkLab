// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Activatable.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UActivatable : public UInterface
{
	GENERATED_BODY()
};

// Represents objects that can be activated on the scene
class DARKLAB_API IActivatable
{
	GENERATED_BODY()

public:
	// Called when the object is activated
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Activatable")
	void Activate();
};
