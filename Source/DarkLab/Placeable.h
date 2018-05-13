// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Placeable.generated.h"

// TODO move somewhere else
// Direction
UENUM(BlueprintType)
enum class EDirectionEnum : uint8
{
	VE_Up	UMETA(DisplayName = "Up"),
	VE_Right 	UMETA(DisplayName = "Right"),
	VE_Down	UMETA(DisplayName = "Down"),
	VE_Left	UMETA(DisplayName = "Left")
};

// TODO move somewhere else
// Represents a rectangular space on the grid
USTRUCT()
struct FRectSpaceStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	int BotLeftX;

	UPROPERTY()
	int BotLeftY;

	UPROPERTY()
	int SizeX;

	UPROPERTY()
	int SizeY;

	ENGINE_API bool operator== (const FRectSpaceStruct& other)
	{
		return BotLeftX == other.BotLeftX && BotLeftY == other.BotLeftY && SizeX == other.SizeX && SizeY == other.SizeY;
	}
	ENGINE_API friend bool operator== (const FRectSpaceStruct& a, const FRectSpaceStruct& b)
	{
		return a.BotLeftX == b.BotLeftX && a.BotLeftY == b.BotLeftY && a.SizeX == b.SizeX && a.SizeY == b.SizeY;
	}

	FRectSpaceStruct()
	{
		SizeX = 1;
		SizeY = 1;
	}
	FRectSpaceStruct(int botLeftX, int botLeftY, int sizeX, int sizeY) : BotLeftX(botLeftX), BotLeftY(botLeftY)
	{
		SizeX = sizeX > 0 ? sizeX : 1;
		SizeY = sizeY > 0 ? sizeY : 1;
	}
};

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

	// Tries to set new size of the object in cells, returns success
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Placeable")
	bool SetSize(const FIntVector size);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Placeable")
	bool SetSizeXY(const int x, const int y);

	// Places the object on the map, using bottom left corner
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Placeable")
	void Place(const FIntVector botLeftLoc, const EDirectionEnum direction);
};