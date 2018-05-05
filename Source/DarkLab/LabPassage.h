// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Placeable.h"

class LabRoom;

// Represents a pass between two rooms in the laboratory
class DARKLAB_API LabPassage
{
public:
	// Passage's location and width
	int BotLeftLocX = 0;
	int BotLeftLocY = 0;
	int Width = 4;

	// Direction of the passage (not along its width but along player's path)
	EDirectionEnum GridDirection;

	// TODO make into a comprehensive enum instead of bool
	// True if there's a door
	bool bIsDoor = false;

	// The color of the door if it's a door. This also serves as code
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Passage")
	FLinearColor Color = FLinearColor::White;

	// What the passage connects
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Passage")
	LabRoom* From;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Passage")
	LabRoom* To;

public:
	// Sets default values
	LabPassage(int botLeftLocX, int botLeftLocY, EDirectionEnum direction, LabRoom* from = nullptr, LabRoom* to = nullptr, bool isDoor = false, FLinearColor color = FLinearColor::White, int width = 4);

	// Called on destruction
	~LabPassage();
};