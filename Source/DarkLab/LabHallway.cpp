// Fill out your copyright notice in the Description page of Project Settings.

#include "LabHallway.h"
#include "LabPassage.h"

// Sets default values
LabHallway::LabHallway(int botLeftX, int botLeftY, EDirectionEnum direction, int length, int width, LabRoom * from, LabRoom * to, int enterWidth, int exitWidth, LabRoom* outer) : LabHallway(botLeftX, botLeftY, direction, length, width, from, to, false, false, FLinearColor::White, FLinearColor::White, enterWidth, exitWidth, outer)
{ }
LabHallway::LabHallway(int botLeftX, int botLeftY, EDirectionEnum direction, int length, int width, LabRoom * from, LabRoom * to, bool enterIsDoor, bool exitIsDoor, FLinearColor enterColor, FLinearColor exitColor, int enterWidth, int exitWidth, LabRoom* outer) : LabRoom(botLeftX, botLeftY, (direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down) ? width : length, (direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down) ? length : width, outer), From(from), To(to), bEnterIsDoor(enterIsDoor), bExitIsDoor(exitIsDoor), EnterColor(enterColor), ExitColor(exitColor)
{
	EnterWidth = enterWidth;
	ExitWidth = exitWidth;

	switch(direction)
	{
	case EDirectionEnum::VE_Up:
		AddPassage(BotLeftX + (width - EnterWidth) / 2, BotLeftY, direction, from, enterIsDoor, enterColor, enterWidth);
		AddPassage(BotLeftX + (width - ExitWidth) / 2, BotLeftY + length - 1, direction, to, exitIsDoor, exitColor, exitWidth);
		break;
	case EDirectionEnum::VE_Down:
		AddPassage(BotLeftX + (width - EnterWidth) / 2, BotLeftY + length - 1, direction, from, enterIsDoor, enterColor, enterWidth);
		AddPassage(BotLeftX + (width - ExitWidth) / 2, BotLeftY, direction, to, exitIsDoor, exitColor, exitWidth);
		break;
	case EDirectionEnum::VE_Right:
		AddPassage(BotLeftX, BotLeftY + (width - EnterWidth) / 2, direction, from, enterIsDoor, enterColor, enterWidth);
		AddPassage(BotLeftX + length - 1, BotLeftY + (width - ExitWidth) / 2, direction, to, exitIsDoor, exitColor, exitWidth);
		break;
	case EDirectionEnum::VE_Left:
		AddPassage(BotLeftX + length - 1, BotLeftY + (width - EnterWidth) / 2, direction, from, enterIsDoor, enterColor, enterWidth);
		AddPassage(BotLeftX, BotLeftY + (width - ExitWidth) / 2, direction, to, exitIsDoor, exitColor, exitWidth);
		break;
	}
}

// Called on destruction
LabHallway::~LabHallway()
{
	LabRoom::~LabRoom();
}