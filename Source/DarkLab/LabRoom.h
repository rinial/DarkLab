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
	int BotLeftLocX = 0;
	int BotLeftLocY = 0;
	int SizeX = 4;
	int SizeY = 4;

	// Used for rooms inside rooms
	bool bIsInner = false;

	// Passages from this room
	TArray<LabPassage*> Passages;

public:
	// Adds a passage to/from this room
	// Returns false if it's not possible
	bool AddPassage(LabPassage* passage);
	bool AddPassage(int botLeftLocX, int botLeftLocY, EDirectionEnum direction, LabRoom* other = nullptr, int width = 4);
	bool AddPassage(int botLeftLocX, int botLeftLocY, EDirectionEnum direction, LabRoom* other, bool isDoor, FLinearColor color = FLinearColor::White, int width = 4); 

public:
	// Sets default values
	LabRoom(int botLeftLocX, int botLeftLocY, int sizeX, int sizeY, bool isInner = false);

	// Called on destruction
	~LabRoom();
};