// Fill out your copyright notice in the Description page of Project Settings.

#include "LabRoom.h"
#include "LabPassage.h"

// Adds a passage to/from this room
// Returns nullptr if it's not possible
LabPassage* LabRoom::AddPassage(LabPassage * passage)
{
	if (!passage)
		return passage;

	// In this case passage was already created with this room as parameter before (probably from other AddPassage method) and we just finish initialization
	if (passage->From == this || passage->To == this)
	{
		// We don't check whether the passage is in the right place
		// We leave that on the other AddPassage method that will call Passage constructor
		// which in turn will call this method
		Passages.AddUnique(passage);
		return passage;
	}
	// In this case passage was created without current room
	else
	{
		// We dont add passage if it already has both To and From
		if (passage->From != nullptr && passage->To != nullptr)
			return nullptr;

		// We need to understand if this passage leads from or to this room
		if (passage->From == nullptr && passage->To == nullptr)
		{
			bool fromThis = LeadsFromThisRoom(passage->BotLeftX, passage->BotLeftY, passage->GridDirection);
			if (fromThis)
				passage->From = this;
			else
				passage->To = this;
		}
		else
		{
			if (passage->From != nullptr && passage->To == nullptr)
				passage->To = this;
			if (passage->From == nullptr && passage->To != nullptr)
				passage->From = this;
		}

		Passages.AddUnique(passage);
		return passage;
	}
}
LabPassage* LabRoom::AddPassage(int botLeftX, int botLeftY, EDirectionEnum direction, LabRoom * other, int width)
{
	return AddPassage(botLeftX, botLeftY, direction, other, false, FLinearColor::White, width);
}
LabPassage* LabRoom::AddPassage(int botLeftX, int botLeftY, EDirectionEnum direction, LabRoom * other, bool isDoor, FLinearColor color, int width)
{
	if (width < 2)
		return nullptr;

	if (direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down)
	{
		// if it's not in bottom or top wall of the room
		if (botLeftY != BotLeftY && botLeftY != BotLeftY + SizeY - 1)
			return nullptr;

		// if it goes outside the wall or includes room corners
		if (botLeftX < BotLeftX + 1 || botLeftX + width - 1 > BotLeftX + SizeX - 2)
			return nullptr;
	}
	else
	{
		// if it's not in left or right wall of the room
		if (botLeftX != BotLeftX && botLeftX != BotLeftX + SizeX - 1)
			return nullptr;

		// if it goes outside the wall or includes room corners
		if (botLeftY < BotLeftY + 1 || botLeftY + width - 1 > BotLeftY + SizeY - 2)
			return nullptr;
	}
	// At this point passage should be considered ok

	// We find out if it leads out of this room or into it
	bool fromThis = LeadsFromThisRoom(botLeftX, botLeftY, direction);

	LabPassage* passage = new LabPassage(botLeftX, botLeftY, direction, fromThis ? this : other, fromThis ? other : this, isDoor, color, width);
	// We don't need to add it to Passages, it will be added from LabPassage constructor

	return passage;
}

// Returns true if direction is from this room, not into it
bool LabRoom::LeadsFromThisRoom(int botLeftX, int botLeftY, EDirectionEnum direction)
{
	bool fromThis = true;
	if ((direction == EDirectionEnum::VE_Up && botLeftY == BotLeftY) ||
		(direction == EDirectionEnum::VE_Down && botLeftY == BotLeftY + SizeY - 1) ||
		(direction == EDirectionEnum::VE_Right && botLeftX == BotLeftX) ||
		(direction == EDirectionEnum::VE_Left && botLeftX == BotLeftX + SizeX - 1))
		fromThis = false;
	return fromThis;
}

// Sets default values
LabRoom::LabRoom(int botLeftX, int botLeftY, int sizeX, int sizeY, LabRoom* outer) : BotLeftX(botLeftX), BotLeftY(botLeftY), OuterRoom(outer)
{
	SizeX = sizeX > 4 ? sizeX : 4;
	SizeY = sizeY > 4 ? sizeY : 4;
}
LabRoom::LabRoom(FRectSpaceStruct locSize, LabRoom * outer) : LabRoom(locSize.BotLeftX, locSize.BotLeftY, locSize.SizeX, locSize.SizeY, outer)
{ }

// Called on destruction
LabRoom::~LabRoom()
{
	for (int i = Passages.Num() - 1; i >= 0; --i)
	{
		LabPassage* temp = Passages[i];
		Passages.RemoveAt(i);
		if (!temp)
			continue;

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