// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LabRoom.h"

// Represents a straight hallway in the laboratory
class DARKLAB_API LabHallway : public LabRoom
{
public:
	// TODO change into getters
	// Passages' properties
	int EnterWidth = 2;
	int ExitWidth = 2;
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
	LabHallway(int botLeftLocX, int botLeftLocY, EDirectionEnum direction, int length, int width = 8, LabRoom* from = nullptr, LabRoom* to = nullptr, int enterWidth = 4, int exitWidth = 4, bool isInner = false);
	LabHallway(int botLeftLocX, int botLeftLocY, EDirectionEnum direction, int length, int width, LabRoom* from, LabRoom* to, bool enterIsDoor, bool exitIsDoor, FLinearColor enterColor = FLinearColor::White, FLinearColor exitColor = FLinearColor::White, int enterWidth = 4, int exitWidth = 4, bool isInner = false);

	// Called on destruction
	~LabHallway();
};