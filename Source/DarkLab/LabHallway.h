// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LabRoom.h"

// Represents a straight hallway in the laboratory
class DARKLAB_API LabHallway : public LabRoom
{
public:
	// Passages' properties
	int EnterWidth = 3;
	int ExitWidth = 3;
	bool bEnterIsDoor = false;
	bool bExitIsDoor = false;
	FLinearColor EnterColor = FLinearColor::White;
	FLinearColor ExitColor = FLinearColor::White;

	// What the hallway connects
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hallway")
	LabRoom* From;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hallway")
	LabRoom* To;

public:
	// Sets default values
	LabHallway(int botLeftLocX, int botLeftLocY, EDirectionEnum direction, int length, int width = 8, LabRoom* from = nullptr, LabRoom* to = nullptr, bool enterIsDoor = false, bool exitIsDoor = false, FLinearColor enterColor = FLinearColor::White, FLinearColor exitColor = FLinearColor::White, int enterWidth = 4, int exitWidth = 4);

	// Called on destruction
	~LabHallway();
};