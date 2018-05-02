// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Placeable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlaceable : public UInterface
{
	GENERATED_BODY()
};

// Represents objects that can be placed on the map or are parts of the map itself
class DARKLAB_API IPlaceable
{
	GENERATED_BODY()

public:
	// Returns the size of the object in cells
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Placeable")
	FIntVector GetSize();

	// Places the object on the map, using bottom left corner
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Placeable")
	void Place(FIntVector botLeftLoc);
};