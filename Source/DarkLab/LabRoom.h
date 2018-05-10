// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Placeable.h"

class LabPassage;

// Represents a room in the laboratory
// Hallways and forks are also technically rooms
class DARKLAB_API LabRoom
{
public:
	// Room's location and size
	int BotLeftX = 0;
	int BotLeftY = 0;
	int SizeX = 4;
	int SizeY = 4;

	// Used for rooms inside rooms
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room")
	LabRoom* OuterRoom = nullptr;

	// Passages from this room
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room")
	TArray<LabPassage*> Passages;

public:
	// Adds a passage to/from this room
	// Returns false if it's not possible
	LabPassage* AddPassage(LabPassage* passage);
	LabPassage* AddPassage(int botLeftX, int botLeftY, EDirectionEnum direction, int width = 4);
	LabPassage* AddPassage(int botLeftX, int botLeftY, EDirectionEnum direction, bool isDoor, FLinearColor color = FLinearColor::White, int width = 4);
	LabPassage* AddPassage(int botLeftX, int botLeftY, EDirectionEnum direction, LabRoom* other, int width = 4);
	LabPassage* AddPassage(int botLeftX, int botLeftY, EDirectionEnum direction, LabRoom* other, bool isDoor, FLinearColor color = FLinearColor::White, int width = 4);

private:
	// Returns true if passage is from this room, not into it
	bool LeadsFromThisRoom(int botLeftX, int botLeftY, EDirectionEnum direction);

public:
	// Sets default values
	LabRoom(int botLeftX, int botLeftY, int sizeX, int sizeY, LabRoom* outer = nullptr);
	LabRoom(FRectSpaceStruct locSize, LabRoom* outer = nullptr);

	// Called on destruction
	~LabRoom();
};