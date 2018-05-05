// Fill out your copyright notice in the Description page of Project Settings.

#include "LabHallway.h"
#include "LabPassage.h"

// Sets default values
LabHallway::LabHallway(int botLeftLocX, int botLeftLocY, EDirectionEnum direction, int length, int width, LabRoom * from, LabRoom * to, bool enterIsDoor, bool exitIsDoor, FLinearColor enterColor, FLinearColor exitColor, int enterWidth, int exitWidth) : LabRoom(botLeftLocX, botLeftLocY, (direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down) ? width : length, (direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down) ? length : width), From(from), To(to), bEnterIsDoor(enterIsDoor), bExitIsDoor(exitIsDoor), EnterColor(enterColor), ExitColor(exitColor)
{
	if (enterWidth > 2)
		EnterWidth = enterWidth;
	if (exitWidth > 2)
		ExitWidth = exitWidth;

	switch(direction)
	{
	case EDirectionEnum::VE_Up:
		AddPassage(BotLeftLocX + (width - EnterWidth) / 2, BotLeftLocY, direction, from, enterIsDoor, enterColor, enterWidth);
		AddPassage(BotLeftLocX + (width - ExitWidth) / 2, BotLeftLocY + length - 1, direction, to, exitIsDoor, exitColor, exitWidth);
		break;
	case EDirectionEnum::VE_Down:
		AddPassage(BotLeftLocX + (width - EnterWidth) / 2, BotLeftLocY + length - 1, direction, from, enterIsDoor, enterColor, enterWidth);
		AddPassage(BotLeftLocX + (width - ExitWidth) / 2, BotLeftLocY, direction, to, exitIsDoor, exitColor, exitWidth);
		break;
	case EDirectionEnum::VE_Right:
		AddPassage(BotLeftLocX, BotLeftLocY + (width - EnterWidth) / 2, direction, from, enterIsDoor, enterColor, enterWidth);
		AddPassage(BotLeftLocX + length - 1, BotLeftLocY + (width - ExitWidth) / 2, direction, to, exitIsDoor, exitColor, exitWidth);
		break;
	case EDirectionEnum::VE_Left:
		AddPassage(BotLeftLocX + length - 1, BotLeftLocY + (width - EnterWidth) / 2, direction, from, enterIsDoor, enterColor, enterWidth);
		AddPassage(BotLeftLocX, BotLeftLocY + (width - ExitWidth) / 2, direction, to, exitIsDoor, exitColor, exitWidth);
		break;
	}
}

// Called on destruction
LabHallway::~LabHallway()
{
	LabRoom::~LabRoom();
}