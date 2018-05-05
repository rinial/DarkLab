// Fill out your copyright notice in the Description page of Project Settings.

#include "LabRoom.h"
#include "LabPassage.h"

// Adds a passage to/from this room
// Returns false if it's not possible
bool LabRoom::AddPassage(LabPassage * passage)
{
	if(passage->From != this && passage->To != this)
		return false;

	// We don't check whether the passage is in the right place
	// We leave that on the other AddPassage method that will call Passage constructor
	// which in turn will call this method
	Passages.Add(passage);
	return true;
}
bool LabRoom::AddPassage(int botLeftLocX, int botLeftLocY, EDirectionEnum direction, LabRoom * other, bool isDoor, FLinearColor color, int width)
{
	if (width < 3)
		return false;

	if (direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down)
	{
		// if it's not in bottom or top wall of the room
		if (botLeftLocY != BotLeftLocY && botLeftLocY != BotLeftLocY + SizeY - 1)
			return false;

		// if it goes outside the wall or includes room corners
		if (botLeftLocX < BotLeftLocX + 1 || botLeftLocX + width - 1 > BotLeftLocX + SizeX - 2)
			return false;
	}
	else
	{
		// if it's not in left or right wall of the room
		if (botLeftLocX != BotLeftLocX && botLeftLocX != BotLeftLocX + SizeX - 1)
			return false;

		// if it goes outside the wall or includes room corners
		if (botLeftLocY < BotLeftLocY + 1 || botLeftLocY + width - 1 > BotLeftLocY + SizeY - 2)
			return false;
	}
	// At this point passage should be considered ok

	// We find out if it leads out of this room or into it
	bool fromThis = true;
	if ((direction == EDirectionEnum::VE_Up && botLeftLocY == BotLeftLocY) ||
		(direction == EDirectionEnum::VE_Down && botLeftLocY == BotLeftLocY + SizeY - 1) ||
		(direction == EDirectionEnum::VE_Right && botLeftLocX == BotLeftLocX) ||
		(direction == EDirectionEnum::VE_Left && botLeftLocX == BotLeftLocX + SizeX - 1))
		fromThis = false;

	LabPassage* passage = new LabPassage(botLeftLocX, botLeftLocY, direction, fromThis ? this : other, fromThis ? other : this, isDoor, color, width);
	// We don't need to add it to Passages, it will be added from LabPassage constructor

	return true;
}

// Sets default values
LabRoom::LabRoom(int botLeftLocX, int botLeftLocY, int sizeX, int sizeY) : BotLeftLocX(botLeftLocX), BotLeftLocY(botLeftLocY)
{
	if (sizeX > 4)
		SizeX = sizeX;
	if (sizeY > 4)
		SizeY = sizeY;
}

// Called on destruction
LabRoom::~LabRoom()
{
	for (int i = Passages.Num() - 1; i >= 0; --i)
	{
		LabPassage* temp = Passages[i];
		Passages.RemoveAt(i);
		// We delete passage if it isn't connected to some other room
		// We delete passage's pointer to this room otherwise
		if (temp->From == this)
		{
			if (!temp->To)
				delete temp;
			else
				temp->From = nullptr;
		}
		else if (temp->To == this)
		{
			if (!temp->From)
				delete temp;
			else
				temp->To = nullptr;
		}
		// else
			// This is a weird case that should never happen
			// This room is not responsible for the passage is this DOES happen somehow
	}
}