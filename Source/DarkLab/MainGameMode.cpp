// Fill out your copyright notice in the Description page of Project Settings.

#include "MainGameMode.h"
#include "EngineUtils.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "UObject/UObjectIterator.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"
#include "BasicFloor.h"
#include "BasicWall.h"
#include "BasicDoor.h"
#include "WallLamp.h"
#include "Flashlight.h"
#include "LabPassage.h"
#include "LabRoom.h"
#include "LabHallway.h"
#include "Algo/BinarySearch.h"

// Returns the light level and the location of the brightest light
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const bool sixPoints, const float sixPointsRadius, const bool fourMore)
{
	if (!actor)
		return 0.0f;
	return GetLightingAmount(lightLoc, actor, actor->GetActorLocation(), sixPoints, sixPointsRadius, fourMore);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const FVector location, const bool sixPoints, const float sixPointsRadius, const bool fourMore)
{
	return GetLightingAmount(lightLoc, nullptr, location, sixPoints, sixPointsRadius, fourMore);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const TArray<FVector> locations)
{
	return GetLightingAmount(lightLoc, nullptr, locations);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const FVector location, const bool sixPoints, const float sixPointsRadius, const bool fourMore)
{
	TArray<FVector> locations;
	locations.Add(location);
	if (sixPoints)
	{
		// We add six locations around the point
		locations.Add(location + FVector::UpVector * sixPointsRadius);
		locations.Add(location - FVector::UpVector * sixPointsRadius);
		locations.Add(location + FVector::RightVector * sixPointsRadius);
		locations.Add(location - FVector::RightVector * sixPointsRadius);
		locations.Add(location + FVector::ForwardVector * sixPointsRadius);
		locations.Add(location - FVector::ForwardVector * sixPointsRadius);
		
		// We add four more locations around the point (diagonally)
		if (fourMore)
		{
			FVector temp = FVector(1, 1, 0);
			temp.Normalize();
			locations.Add(location + temp * sixPointsRadius);
			locations.Add(location - temp * sixPointsRadius);
			temp = FVector(-1, 1, 0);
			temp.Normalize();
			locations.Add(location + temp * sixPointsRadius);
			locations.Add(location - temp * sixPointsRadius);
		}
	}
	return GetLightingAmount(lightLoc, actor, locations);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const TArray<FVector> locations)
{
	FCollisionQueryParams params = FCollisionQueryParams(FName(TEXT("LightTrace")), true);
	// TODO delete? we already set up visibility channel
	// Add actor and all of its components as ignored
	if (actor)
	{
		params.AddIgnoredActor(actor);
		TInlineComponentArray<UActorComponent*> components;
		actor->GetComponents(components, true);
		for (UActorComponent* component : components)
			params.AddIgnoredComponent(Cast<UPrimitiveComponent>(component));
	}

	float result = 0.0f;

	// First we find all the point lights (spot lights count as point lights)
	TArray<UPointLightComponent*> pointLights;
	UWorld* gameWorld = GetWorld();
	for (TObjectIterator<UPointLightComponent> Itr; Itr; ++Itr)
	{
		// World Check
		if (Itr->GetWorld() != gameWorld)
			continue;

		// We don't care about invisible lights
		if ((!Itr->IsVisible()) || Itr->bHiddenInGame)
			continue;

		pointLights.Add(*Itr);
	}

	// We find local results for all locations
	for (FVector location : locations)
	{
		DrawDebugPoint(gameWorld, location, 5, FColor::Red);

		// This will be used for the spot lights
		FBoxSphereBounds bounds = FBoxSphereBounds(location, FVector(1, 1, 1), 1);

		// We take the highest local result among lights
		for (UPointLightComponent* lightComp : pointLights)
		{
			// First we check if it's a spotlight and whether location is in the cone
			USpotLightComponent* spotLight = Cast<USpotLightComponent>(lightComp);
			if (spotLight && !spotLight->AffectsBounds(bounds))
				continue;

			FVector lightLocation = lightComp->GetComponentLocation();
			float distance = FVector::Dist(location, lightLocation);
			float lightRadius = lightComp->AttenuationRadius;
			// UE_LOG(LogTemp, Warning, TEXT("dist: %f, rad: %f"), distance, lightRadius);
			if (distance > lightRadius)
				continue;

			bool bHit = GetWorld()->LineTraceTestByChannel(lightLocation, location, ECC_Visibility, params);
			// TODO delete if affects performance too much
			// This is a line trace in a different direction helpful in cases when light is positioned inside something like a wall which is ignored by the line trace
			if(!bHit)
				bHit =  GetWorld()->LineTraceTestByChannel(location, lightLocation, ECC_Visibility, params);

			// If location could be lit
			if (!bHit)
			{
				DrawDebugLine(gameWorld, location, lightLocation, FColor::Cyan);

				// 1 if near the edge of light, 0 if in center
				float temp = distance / lightRadius;
				// We take into account the inverse squared falloff
				temp = FMath::Pow(temp, 0.5f);
				// Now it's 0 near the edge and 1 in center
				temp = 1 - temp;
				// Finally we take intensity into account
				// temp *= lightComp->Intensity * spotK;

				// We also take color into account
				FLinearColor lightColor = lightComp->GetLightColor();
				temp *= FMath::Pow(lightColor.R * lightColor.R + lightColor.G * lightColor.G + lightColor.B * lightColor.B / 3.0f, 0.5f);

				// UE_LOG(LogTemp, Warning, TEXT("%f"), temp);
				// It always counts the brightest light
				if (temp > result)
				{
					result = temp;
					lightLoc = lightLocation;
				}
			}
		}
	}

	// UE_LOG(LogTemp, Warning, TEXT("Final %f"), result);
	return result;
}

// Places an object on the map
// TODO return false if can't place?
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftX, const int botLeftY, const EDirectionEnum direction)
{
	PlaceObject(object, FIntVector(botLeftX, botLeftY, 0), direction);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftX, const int botLeftY, const int sizeX, const int sizeY)
{
	PlaceObject(object, FIntVector(botLeftX, botLeftY, 0), EDirectionEnum::VE_Up, sizeX, sizeY);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, const int sizeZ)
{
	PlaceObject(object, FIntVector(botLeftX, botLeftY, 0), EDirectionEnum::VE_Up, sizeX, sizeY, sizeZ);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftX, const int botLeftY, const EDirectionEnum direction, const int sizeX, const int sizeY)
{
	PlaceObject(object, FIntVector(botLeftX, botLeftY, 0), direction, sizeX, sizeY);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftX, const int botLeftY, const EDirectionEnum direction, const int sizeX, const int sizeY, const int sizeZ)
{
	PlaceObject(object, FIntVector(botLeftX, botLeftY, 0), direction, sizeX, sizeY, sizeZ);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc, const EDirectionEnum direction)
{
	UObject* obj = object->_getUObject();
	object->Execute_Place(obj, botLeftLoc, direction);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc, const int sizeX, const int sizeY)
{
	PlaceObject(object, botLeftLoc, EDirectionEnum::VE_Up, sizeX, sizeY);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc, const int sizeX, const int sizeY, const int sizeZ)
{
	PlaceObject(object, botLeftLoc, EDirectionEnum::VE_Up, sizeX, sizeY, sizeZ);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc, const EDirectionEnum direction, const int sizeX, const int sizeY)
{
	UObject* obj = object->_getUObject();
	object->Execute_SetSizeXY(obj, sizeX, sizeY);
	object->Execute_Place(obj, botLeftLoc, direction);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc, const EDirectionEnum direction, const int sizeX, const int sizeY, const int sizeZ)
{
	UObject* obj = object->_getUObject();
	object->Execute_SetSize(obj, FIntVector(sizeX, sizeY, sizeZ));
	object->Execute_Place(obj, botLeftLoc, direction);
}

// Gets the pool for the object/class
TArray<TScriptInterface<IDeactivatable>>& AMainGameMode::GetCorrectPool(TScriptInterface<IDeactivatable> object)
{
	UClass* cl = object->_getUObject()->GetClass();

	return GetCorrectPool(cl);
}
TArray<TScriptInterface<IDeactivatable>>& AMainGameMode::GetCorrectPool(UClass * cl)
{
	if (cl == BasicFloorBP)
		return BasicFloorPool;
	if (cl == BasicWallBP)
		return BasicWallPool;
	if (cl == BasicDoorBP)
		return BasicDoorPool;
	if (cl == WallLampBP)
		return WallLampPool;
	if (cl == FlashlightBP)
		return FlashlightPool;
	return DefaultPool;
}

// Deactivates and adds to a pool
void AMainGameMode::PoolObject(TScriptInterface<IDeactivatable> object)
{
	object->Execute_SetActive(object->_getUObject(), false);
	GetCorrectPool(object).Add(object);
}
void AMainGameMode::PoolObjects(TArray<TScriptInterface<IDeactivatable>>& objects)
{
	for (TScriptInterface<IDeactivatable> object : objects)
		PoolObject(object);
}

// Pool full parts of the lab
void AMainGameMode::PoolRoom(LabRoom * room)
{
	if (!SpawnedRoomObjects.Contains(room))
		return;

	PoolObjects(SpawnedRoomObjects[room]);
	SpawnedRoomObjects.Remove(room);
	for (LabPassage* passage : room->Passages)
	{
		if (!passage)
			continue;

		// We pool passage if it isn't connected to some other room
		if (passage->From == room)
		{
			if (!passage->To)
				PoolPassage(passage);
		}
		else if (passage->To == room)
		{
			if (!passage->From)
				PoolPassage(passage);
		}
		// else
		// This is a weird case that should never happen
		// Room is not responsible for the passage pooling is this DOES happen somehow
	}
	DeallocateSpace(room);
	delete room;
}
void AMainGameMode::PoolPassage(LabPassage* passage)
{
	if (!SpawnedPassageObjects.Contains(passage))
		return;

	PoolObjects(SpawnedPassageObjects[passage]);
	SpawnedPassageObjects.Remove(passage);
	// We don't delete passage from here as it's deleted during room's destruction
}

// Tries to find a poolable object in a specified array
UObject* AMainGameMode::TryGetPoolable(UClass* cl)
{
	TArray<TScriptInterface<IDeactivatable>>& pool = GetCorrectPool(cl);

	if (pool.Num() == 0)
		return nullptr;

	int index = -1;
	TScriptInterface<IDeactivatable> object;
	if (pool != DefaultPool)
		index = 0;
	else
	{
		for (int i = 0; i < pool.Num(); ++i)
		{
			TScriptInterface<IDeactivatable> temp = pool[i];
			if (temp->_getUObject()->GetClass() == cl)
			{
				index = i;
				break;
			}
		}
	}

	if(index < 0)
		return nullptr;

	object = pool[index];
	UObject* obj = object->_getUObject();
	object->Execute_SetActive(obj, true);
	pool.RemoveAt(index);
	return obj;
}

// Spawn specific objects
ABasicFloor* AMainGameMode::SpawnBasicFloor(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabRoom* room)
{
	ABasicFloor* floor = Cast<ABasicFloor>(TryGetPoolable(BasicFloorBP));
	if (!floor)
		floor = GetWorld()->SpawnActor<ABasicFloor>(BasicFloorBP);

	PlaceObject(floor, botLeftX, botLeftY, sizeX, sizeY);

	if (room && SpawnedRoomObjects.Contains(room))
		SpawnedRoomObjects[room].Add(floor);

	// UE_LOG(LogTemp, Warning, TEXT("Spawned a basic floor"));

	return floor;
}
ABasicFloor* AMainGameMode::SpawnBasicFloor(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabPassage* passage)
{
	ABasicFloor* floor = SpawnBasicFloor(botLeftX, botLeftY, sizeX, sizeY);
	if (passage && SpawnedPassageObjects.Contains(passage))
		SpawnedPassageObjects[passage].Add(floor);

	return floor;
}
ABasicWall* AMainGameMode::SpawnBasicWall(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabRoom* room)
{
	ABasicWall* wall = Cast<ABasicWall>(TryGetPoolable(BasicWallBP));
	if (!wall)
		wall = GetWorld()->SpawnActor<ABasicWall>(BasicWallBP);

	PlaceObject(wall, botLeftX, botLeftY, sizeX, sizeY);

	if (room && SpawnedRoomObjects.Contains(room))
		SpawnedRoomObjects[room].Add(wall);

	// UE_LOG(LogTemp, Warning, TEXT("Spawned a basic wall"));

	return wall;
}
ABasicDoor * AMainGameMode::SpawnBasicDoor(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color, const int width, LabPassage* passage)
{
	ABasicDoor* door = Cast<ABasicDoor>(TryGetPoolable(BasicDoorBP));
	if (!door)
		door = GetWorld()->SpawnActor<ABasicDoor>(BasicDoorBP);

	door->DoorColor = color; // Sets wall's color
	PlaceObject(door, botLeftX, botLeftY, direction, width);

	if (passage && SpawnedPassageObjects.Contains(passage))
		SpawnedPassageObjects[passage].Add(door);

	// UE_LOG(LogTemp, Warning, TEXT("Spawned a basic door"));

	return door;
}
AWallLamp * AMainGameMode::SpawnWallLamp(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color, const int width, LabRoom* room)
{
	AWallLamp* lamp = Cast<AWallLamp>(TryGetPoolable(WallLampBP));
	if (!lamp)
		lamp = GetWorld()->SpawnActor<AWallLamp>(WallLampBP);

	lamp->Reset(); // Disables light if it was on
	lamp->SetColor(color); // Sets correct color
	PlaceObject(lamp, botLeftX, botLeftY, direction, width);

	if (room && SpawnedRoomObjects.Contains(room))
		SpawnedRoomObjects[room].Add(lamp);

	UE_LOG(LogTemp, Warning, TEXT("Spawned a wall lamp"));

	return lamp;
}
AFlashlight* AMainGameMode::SpawnFlashlight(const int botLeftX, const int botLeftY, const EDirectionEnum direction)
{
	AFlashlight* flashlight = Cast<AFlashlight>((TryGetPoolable(FlashlightBP)));
	if (!flashlight)
		flashlight = GetWorld()->SpawnActor<AFlashlight>(FlashlightBP);

	flashlight->Reset(); // Disables light if it was on
	PlaceObject(flashlight, botLeftX, botLeftY, direction);

	UE_LOG(LogTemp, Warning, TEXT("Spawned a flashlight"));

	return flashlight;
}

// Spawn full parts of the lab
void AMainGameMode::SpawnRoom(LabRoom * room)
{
	if (!room)
		return;

	// We don't spawn one room twice
	if (SpawnedRoomObjects.Contains(room))
		return;

	SpawnedRoomObjects.Add(room);

	// Spawning floor
	// Doesn't include walls and passages
	if(!room->OuterRoom)
		SpawnBasicFloor(room->BotLeftX + 1, room->BotLeftY + 1, room->SizeX - 2, room->SizeY - 2, room);

	// For spawning walls 
	TArray<int> leftWallPositions;
	TArray<int> topWallPositions;
	TArray<int> rightWallPositions;
	TArray<int> bottomWallPositions;

	leftWallPositions.Add(0);
	leftWallPositions.Add(room->SizeY - 1);

	topWallPositions.Add(0);
	topWallPositions.Add(room->SizeX - 1);

	rightWallPositions.Add(0);
	rightWallPositions.Add(room->SizeY - 1);

	bottomWallPositions.Add(0);
	bottomWallPositions.Add(room->SizeX - 1);

	// Add positions for spawning walls based on passages in walls
	// Also spawn doors
	for (LabPassage* passage : room->Passages)
	{
		// Left wall
		if (passage->BotLeftX == room->BotLeftX)
		{
			leftWallPositions.Add(passage->BotLeftY - 1 - room->BotLeftY);
			leftWallPositions.Add(passage->BotLeftY + passage->Width - room->BotLeftY);
		}
		// Top wall
		else if (passage->BotLeftY == room->BotLeftY + room->SizeY - 1)
		{
			topWallPositions.Add(passage->BotLeftX - 1 - room->BotLeftX);
			topWallPositions.Add(passage->BotLeftX + passage->Width - room->BotLeftX);
		}
		// Right wall
		else if (passage->BotLeftX == room->BotLeftX + room->SizeX - 1)
		{
			rightWallPositions.Add(passage->BotLeftY - 1 - room->BotLeftY);
			rightWallPositions.Add(passage->BotLeftY + passage->Width - room->BotLeftY);
		}
		// Bottom wall
		else if (passage->BotLeftY == room->BotLeftY)
		{
			bottomWallPositions.Add(passage->BotLeftX - 1 - room->BotLeftX);
			bottomWallPositions.Add(passage->BotLeftX + passage->Width - room->BotLeftX);
		}

		// Spawn the passage
		SpawnPassage(passage, room);
	}

	leftWallPositions.Sort();
	topWallPositions.Sort();
	rightWallPositions.Sort();
	bottomWallPositions.Sort();

	// Spawning left walls
	for (int i = 0; i + 1 < leftWallPositions.Num(); i += 2)
	{
		int wallLength = leftWallPositions[i + 1] - leftWallPositions[i] + 1;
		SpawnBasicWall(room->BotLeftX, room->BotLeftY + leftWallPositions[i], 1, wallLength, room);
	}
	// Spawning top walls
	for (int i = 0; i + 1 < topWallPositions.Num(); i += 2)
	{
		int wallLength = topWallPositions[i + 1] - topWallPositions[i] + 1;
		SpawnBasicWall(room->BotLeftX + topWallPositions[i], room->BotLeftY + room->SizeY - 1, wallLength, 1, room);
	}
	// Spawning right walls
	for (int i = 0; i + 1 < rightWallPositions.Num(); i += 2)
	{
		int wallLength = rightWallPositions[i + 1] - rightWallPositions[i] + 1;
		SpawnBasicWall(room->BotLeftX + room->SizeX - 1, room->BotLeftY + rightWallPositions[i], 1, wallLength, room);
	}
	// Spawning bottom walls
	for (int i = 0; i + 1 < bottomWallPositions.Num(); i += 2)
	{
		int wallLength = bottomWallPositions[i + 1] - bottomWallPositions[i] + 1;
		SpawnBasicWall(room->BotLeftX + bottomWallPositions[i], room->BotLeftY, wallLength, 1, room);
	}

	UE_LOG(LogTemp, Warning, TEXT("Spawned a room"));
}
void AMainGameMode::SpawnPassage(LabPassage* passage, LabRoom* room)
{
	if (!passage)
		return;

	// We don't spawn one passage twice
	if (SpawnedPassageObjects.Contains(passage))
		return;

	SpawnedPassageObjects.Add(passage);

	// Spawns floor under the passage
	if (!room || (!room->OuterRoom || passage->From == room->OuterRoom || passage->To == room->OuterRoom))
	{
		if (passage->GridDirection == EDirectionEnum::VE_Up || passage->GridDirection == EDirectionEnum::VE_Down)
			SpawnBasicFloor(passage->BotLeftX, passage->BotLeftY, passage->Width, 1, passage);
		else
			SpawnBasicFloor(passage->BotLeftX, passage->BotLeftY, 1, passage->Width, passage);
	}

	// Spawn door if needed
	if (passage->bIsDoor)
	{
		SpawnBasicDoor(passage->BotLeftX, passage->BotLeftY, passage->GridDirection, passage->Color, passage->Width, passage);
	}

	UE_LOG(LogTemp, Warning, TEXT("> Spawned a passage"));
}

// Space is allocated and can't be allocated again
FRectSpaceStruct* AMainGameMode::AllocateSpace(LabRoom * room)
{
	if (!room)
		return nullptr;

	FRectSpaceStruct* temp = AllocateSpace(room->BotLeftX, room->BotLeftY, room->SizeX, room->SizeY);
	if (temp)
		AllocatedRoomSpace.Add(room, temp);

	return temp;
}
FRectSpaceStruct* AMainGameMode::AllocateSpace(int botLeftX, int botLeftY, int sizeX, int sizeY)
{
	FRectSpaceStruct temp = FRectSpaceStruct(botLeftX, botLeftY, sizeX, sizeY);
	return AllocateSpace(temp);
}
FRectSpaceStruct* AMainGameMode::AllocateSpace(FRectSpaceStruct& space)
{
	if (space.SizeX < 1 || space.SizeY < 1)
		return nullptr;

	return &AllocatedSpace[AllocatedSpace.Add(space)];

	//// It's index from AllocatedSpace array
	//int index;
	//if (DeallocatedIndices.Num() > 0)
	//{
	//	index = DeallocatedIndices[0];
	//	DeallocatedIndices.RemoveAt(0);
	//}
	//else
	//	index = AllocatedSpace.AddDefaulted();
	//AllocatedSpace[index] = space;

	//// Now we need to fill the indices arrays while keeping them sorted
	//// For X1
	//int indexWithHigherX1 = Algo::LowerBound(AllocatedX1Indices, index, [this](int ind1, int ind2) { return AllocatedSpace[ind1].BotLeftX < AllocatedSpace[ind2].BotLeftX; });
	//if (indexWithHigherX1 == INDEX_NONE)
	//	AllocatedX1Indices.Add(index);
	//else
	//	AllocatedX1Indices.Insert(index, indexWithHigherX1);
	//// For X2
	//int indexWithHigherX2 = Algo::LowerBound(AllocatedX2Indices, index, [this](int ind1, int ind2) { return AllocatedSpace[ind1].BotLeftX + AllocatedSpace[ind1].SizeX < AllocatedSpace[ind2].BotLeftX + AllocatedSpace[ind2].SizeX; });
	//if (indexWithHigherX2 == INDEX_NONE)
	//	AllocatedX2Indices.Add(index);
	//else
	//	AllocatedX2Indices.Insert(index, indexWithHigherX2);
	//// For Y1
	//int indexWithHigherY1 = Algo::LowerBound(AllocatedY1Indices, index, [this](int ind1, int ind2) { return AllocatedSpace[ind1].BotLeftY < AllocatedSpace[ind2].BotLeftY; });
	//if (indexWithHigherY1 == INDEX_NONE)
	//	AllocatedY1Indices.Add(index);
	//else
	//	AllocatedY1Indices.Insert(index, indexWithHigherY1);
	//// For Y2
	//int indexWithHigherY2 = Algo::LowerBound(AllocatedY2Indices, index, [this](int ind1, int ind2) { return AllocatedSpace[ind1].BotLeftY + AllocatedSpace[ind1].SizeY < AllocatedSpace[ind2].BotLeftY + AllocatedSpace[ind2].SizeY; });
	//if (indexWithHigherY2 == INDEX_NONE)
	//	AllocatedY2Indices.Add(index);
	//else
	//	AllocatedY2Indices.Insert(index, indexWithHigherY2);
}
// Space is not allocated anymore
void AMainGameMode::DeallocateSpace(FRectSpaceStruct& space)
{
	AllocatedSpace.Remove(space);
	// DeallocateSpace(space.BotLeftX, space.BotLeftY, space.SizeX, space.SizeY);
}
void AMainGameMode::DeallocateSpace(LabRoom* room)
{
	if (!room)
		return;

	if (AllocatedRoomSpace.Contains(room))
	{
		AllocatedSpace.Remove(*AllocatedRoomSpace[room]);
		AllocatedRoomSpace.Remove(room);
	}
	else
		DeallocateSpace(room->BotLeftX, room->BotLeftY, room->SizeX, room->SizeY);
}
void AMainGameMode::DeallocateSpace(int botLeftX, int botLeftY, int sizeX, int sizeY)
{
	FRectSpaceStruct* temp = AllocatedSpace.FindByPredicate([botLeftX, botLeftY, sizeX, sizeY](FRectSpaceStruct space)
	{
		// ==
		return space.BotLeftX == botLeftX && space.BotLeftY == botLeftY && space.SizeX == sizeX && space.SizeY == sizeY;
	});
	if (!temp)
		return;

	AllocatedSpace.Remove(*temp);
}

// Returns true if there is free rectangular space
bool AMainGameMode::RectSpaceIsFree(FRectSpaceStruct& space)
{
	return RectSpaceIsFree(space.BotLeftX, space.BotLeftY, space.SizeX, space.SizeY);
}
bool AMainGameMode::RectSpaceIsFree(int botLeftX, int botLeftY, int sizeX, int sizeY)
{
	if (sizeX < 1 || sizeY < 1)
		return false;

	bool intersected = AllocatedSpace.ContainsByPredicate([botLeftX, botLeftY, sizeX, sizeY](FRectSpaceStruct space)
	{
		// Not intersecting on X axis
		if (space.BotLeftX + space.SizeX - 1 <= botLeftX)
			return false;
		if (space.BotLeftX >= botLeftX + sizeX - 1)
			return false;

		// Not intersecting on Y axis
		if (space.BotLeftY + space.SizeY - 1 <= botLeftY)
			return false;
		if (space.BotLeftY >= botLeftY + sizeY - 1)
			return false;

		// Intersecting on both axis
		return true;
	});

	return !intersected;
}

// Tries to create a room and allocate space for it
LabRoom* AMainGameMode::CreateRoom(int botLeftX, int botLeftY, int sizeX, int sizeY)
{
	if(sizeX < 4 || sizeY < 4 || !RectSpaceIsFree(botLeftX, botLeftY, sizeX, sizeY))
		return nullptr;

	LabRoom* room = new LabRoom(botLeftX, botLeftY, sizeX, sizeY);
	AllocateSpace(room);
	return room;
}
LabRoom* AMainGameMode::CreateRoom(FRectSpaceStruct & space)
{
	if (space.SizeX < 4 || space.SizeY < 4 || !RectSpaceIsFree(space))
		return nullptr;

	LabRoom* room = new LabRoom(space);
	FRectSpaceStruct* temp = AllocateSpace(space);
	if (!temp)
		return nullptr;
	AllocatedRoomSpace.Add(room, temp);
	return room;
}

// Sets default values
AMainGameMode::AMainGameMode()
{
	// Find blueprints and save found classes for future spawns
	static ConstructorHelpers::FObjectFinder<UClass> basicFloorBP(TEXT("Class'/Game/Blueprints/BasicFloorBP.BasicFloorBP_C'"));
	if (basicFloorBP.Succeeded())
		BasicFloorBP = basicFloorBP.Object;
	static ConstructorHelpers::FObjectFinder<UClass> basicWallBP(TEXT("Class'/Game/Blueprints/BasicWallBP.BasicWallBP_C'"));
	if (basicWallBP.Succeeded())
		BasicWallBP = basicWallBP.Object;
	static ConstructorHelpers::FObjectFinder<UClass> basicDoorBP(TEXT("Class'/Game/Blueprints/BasicDoorBP.BasicDoorBP_C'"));
	if (basicDoorBP.Succeeded())
		BasicDoorBP = basicDoorBP.Object;
	static ConstructorHelpers::FObjectFinder<UClass> wallLampBP(TEXT("Class'/Game/Blueprints/WallLampBP.WallLampBP_C'"));
	if (wallLampBP.Succeeded())
		WallLampBP = wallLampBP.Object;
	static ConstructorHelpers::FObjectFinder<UClass> flashlightBP(TEXT("Class'/Game/Blueprints/FlashlightBP.FlashlightBP_C'"));
	if (flashlightBP.Succeeded())
		FlashlightBP = flashlightBP.Object;
}

// Called when the game starts or when spawned
void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();


	// TODO shouldn't create, spawn or activate directly from here


	// Generation:

	// 1. Something allocates a room
	LabRoom* startRoom = CreateRoom(-10, -5, 15, 12);

	// 2. We find positions for passages and allocate minimum size
	LabPassage* p1 = startRoom->AddPassage(-10, -3, EDirectionEnum::VE_Left, true);
	LabPassage* p2 = startRoom->AddPassage(-6, 6, EDirectionEnum::VE_Up, true, FLinearColor::Red);
	LabPassage* p3 = startRoom->AddPassage(4, 1, EDirectionEnum::VE_Left, true);
	LabPassage* p4 = startRoom->AddPassage(-3, -5, EDirectionEnum::VE_Up, 7);

	// 2*. We should already find passages between this room and other allocated but not spawn rooms

	// 3. For each passage we find maximum possible distance in three directions
	// TODO

	// 4. For each passage we allocate new rooms (1. may be happening here)
	LabRoom* r1 = CreateRoom(-20, -16, 11, 25);
	LabRoom* r2 = CreateRoom(-8, 6, 10, 10);
	LabRoom* r3 = CreateRoom(4, -2, 10, 20);
	LabRoom* r4 = CreateRoom(-4, -11, 18, 7);

	// 5. We add passages to our new rooms
	if (r1) r1->AddPassage(p1);
	if (r2) r2->AddPassage(p2);
	if (r3) r3->AddPassage(p3);
	if (r4) r4->AddPassage(p4);

	// 6. We spawn our initial room
	SpawnRoom(startRoom);

	// 7. We initialize and spawn other parts of the initial room
	AWallLamp* lamp = SpawnWallLamp(-5, -5, EDirectionEnum::VE_Up, FLinearColor::White, 1, startRoom);
	if (lamp) lamp->Execute_ActivateIndirectly(lamp); // TODO this shouldn't be here
	AFlashlight* flashlight = SpawnFlashlight(0, 0);

	// 8+. We repeat from 1 for other rooms (rn we just spawn)
	SpawnRoom(r1);
	SpawnRoom(r2);
	SpawnRoom(r3);
	SpawnRoom(r4);

	
	// Tests
	/*
	// Testing room pooling
	LabRoom* room1 = CreateRoom(-50, -50, 100, 100);
	room1->AddPassage(9, -50, EDirectionEnum::VE_Down, true);
	SpawnRoom(room1);
	PoolRoom(room1);

	// Testing wall pooling
	PoolObject(SpawnBasicWall(2, 3, 5, 1)); // This makes no sense except for testing

	// Testing hallways
	LabHallway* hallway1 = new LabHallway(4, -4, EDirectionEnum::VE_Right, 46, 4, startRoom, nullptr, 2, 2);
	LabHallway* hallway2 = new LabHallway(-5, -25, EDirectionEnum::VE_Right, 50, 12, nullptr, nullptr, false, true, FLinearColor::White, FLinearColor::Black, 10, 6);
	SpawnRoom(hallway1);
	SpawnRoom(hallway2);
	
	// Testing basic room parts
	SpawnBasicFloor(-20, -20, 40, 40);
	SpawnBasicWall(-7, -5, 1, 9);
	SpawnBasicDoor(-6, 3, EDirectionEnum::VE_Up, FLinearColor::Red);

	// Testing space allocation
	AllocateSpace(1, 1, 3, 2);
	AllocateSpace(7, 3, 2, 2);
	AllocateSpace(3, 2, 3, 3);
	AllocateSpace(8, 2, 2, 2);
	UE_LOG(LogTemp, Warning, TEXT("%d %d %d %d"), AllocatedSpace[0].BotLeftX, AllocatedSpace[0].BotLeftY, AllocatedSpace[0].SizeX, AllocatedSpace[0].SizeY);
	UE_LOG(LogTemp, Warning, TEXT("%d %d %d %d"), AllocatedSpace[1].BotLeftX, AllocatedSpace[1].BotLeftY, AllocatedSpace[1].SizeX, AllocatedSpace[1].SizeY);
	UE_LOG(LogTemp, Warning, TEXT("%d %d %d %d"), AllocatedSpace[2].BotLeftX, AllocatedSpace[2].BotLeftY, AllocatedSpace[2].SizeX, AllocatedSpace[2].SizeY);
	UE_LOG(LogTemp, Warning, TEXT("%d %d %d %d"), AllocatedSpace[3].BotLeftX, AllocatedSpace[3].BotLeftY, AllocatedSpace[3].SizeX, AllocatedSpace[3].SizeY);
	UE_LOG(LogTemp, Warning, TEXT("%s"), (RectSpaceIsFree(2, 3) ? TEXT("Free space 2 3 1 1") : TEXT("No space")));
	UE_LOG(LogTemp, Warning, TEXT("%s"), (RectSpaceIsFree(1, 3, 2, 2) ? TEXT("Free space 1 3 2 2") : TEXT("No space")));
	UE_LOG(LogTemp, Warning, TEXT("%s"), (RectSpaceIsFree(4, 1, 5, 1) ? TEXT("Free space 4 1 5 1") : TEXT("No space")));
	UE_LOG(LogTemp, Warning, TEXT("%s"), (RectSpaceIsFree(6, 1, 2, 2) ? TEXT("Free space 6 1 2 2") : TEXT("No space")));
	UE_LOG(LogTemp, Warning, TEXT("%s"), (RectSpaceIsFree(5, 2) ? TEXT("NO WAY") : TEXT("No space")));
	UE_LOG(LogTemp, Warning, TEXT("%s"), (RectSpaceIsFree(3, 2, 1, 2) ? TEXT("NO WAY") : TEXT("No space")));
	UE_LOG(LogTemp, Warning, TEXT("%s"), (RectSpaceIsFree(4, 1, 4, 2) ? TEXT("NO WAY") : TEXT("No space")));
	UE_LOG(LogTemp, Warning, TEXT("%s"), (RectSpaceIsFree(6, 2, 2, 3) ? TEXT("NO WAY") : TEXT("No space")));
	*/
}

// Called when actor is being removed from the play
void AMainGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UE_LOG(LogTemp, Warning, TEXT("EndPlay called"));

	TArray<LabRoom*> spawnedRooms;
	SpawnedRoomObjects.GetKeys(spawnedRooms);

	// Clear all saved rooms
	for (int i = spawnedRooms.Num() - 1; i >= 0; --i)
		delete spawnedRooms[i];
}

// Called at start of seamless travel, or right before map change for hard travel
void AMainGameMode::StartToLeaveMap()
{
	Super::StartToLeaveMap();

	UE_LOG(LogTemp, Warning, TEXT("StartToLeaveMap called"));
}