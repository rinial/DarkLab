// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Deactivatable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDeactivatable : public UInterface
{
	GENERATED_BODY()
};

// Represents objects that can be deactivated
class DARKLAB_API IDeactivatable
{
	GENERATED_BODY()

public:
	// Called when the object is activated/deactivated (usually for pooling)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Deactivatable")
	void SetActive(const bool active);	

	// Returns true if the object is active
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Deactivatable")
	bool IsActive();
};