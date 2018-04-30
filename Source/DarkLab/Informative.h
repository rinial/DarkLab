// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Informative.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInformative : public UInterface
{
	GENERATED_BODY()
};

// Represents objects that can provide generic information about themselves
class DARKLAB_API IInformative
{
	GENERATED_BODY()

public:
	// Returns object's name
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Informative")
	FText GetName();

	// Returns basic infomation about the object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Informative")
	FText GetBasicInfo();
};