// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicDeactivatableObject.h"
#include "Placeable.h"
#include "BasicPlaceableObject.generated.h"

// Represents objects with physical represention that can be placed on the map
UCLASS()
class DARKLAB_API ABasicPlaceableObject : public ABasicDeactivatableObject, public IPlaceable
{
	GENERATED_BODY()
	
public:
	// Returns the size of the object in cells
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Placeable")
	FIntVector GetSize();
	virtual FIntVector GetSize_Implementation() override;

	// Tries to set new size of the object in cells, returns success
	// Actually sets not BaseSize but actor's scale, which affects GetSize
	// Assumes direction to be Up. Note: should do Place after this
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Placeable")
	bool SetSize(const FIntVector size);
	virtual bool SetSize_Implementation(const FIntVector size) override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Placeable")
	bool SetSizeXY(const int x, const int y);
	virtual bool SetSizeXY_Implementation(const int x, const int y) override;

	// Places the object on the map, using bottom left corner
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Placeable")
	void Place(const FIntVector botLeftLoc, const EDirectionEnum direction);
	virtual void Place_Implementation(const FIntVector botLeftLoc, const EDirectionEnum direction) override;

protected:
	UPROPERTY()
	FIntVector BaseSize = FIntVector(1, 1, 1);

public:
	UPROPERTY()
	EDirectionEnum GridDirection = EDirectionEnum::VE_Up;

	UPROPERTY()
	float ZOffset = 0.0f;
};