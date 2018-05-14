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
#include "DarknessController.h"

// Probabilities
const float AMainGameMode::ConnectToOtherRoomProbability = 1.0f;
const float AMainGameMode::PassageIsDoorProbability = 0.6f;
const float AMainGameMode::DoorIsNormalProbability = 0.95f;
const float AMainGameMode::SpawnFlashlightProbability = 0.4f; // TODO make it lower
const float AMainGameMode::BlueProbability = 0.2f;
const float AMainGameMode::GreenProbability = 0.15f;
const float AMainGameMode::YellowProbability = 0.1f;
const float AMainGameMode::RedProbability = 0.05f;
const float AMainGameMode::BlackProbability = 0.02f;

// Returns true with certain probability
bool AMainGameMode::RandBool(const float probability)
{
	return FMath::FRand() <= probability;
}
// Returns random color with certain probabilities
FLinearColor AMainGameMode::RandColor()
{
	// Blue
	float temp = FMath::FRand();
	if (temp <= BlueProbability)
		return FLinearColor::FromSRGBColor(FColor(30, 144, 239));

	// Green
	temp -= BlueProbability;
	if (temp <= GreenProbability)
		return FLinearColor::Green;

	// Yellow
	temp -= GreenProbability;
	if (temp <= YellowProbability)
		return FLinearColor::Yellow;

	// Red
	temp -= YellowProbability;
	if (temp <= RedProbability)
		return FLinearColor::Red;

	// Black
	temp -= RedProbability;
	if (temp <= BlackProbability)
		return FLinearColor::Black;

	// White
	return FLinearColor::White;
}
// Returns random direction 
EDirectionEnum AMainGameMode::RandDirection()
{
	int direction = FMath::RandRange(0, 3);

	if (direction == 0)
		return EDirectionEnum::VE_Left;
	if (direction == 1)
		return EDirectionEnum::VE_Right;
	if (direction == 2)
		return EDirectionEnum::VE_Down;
	return EDirectionEnum::VE_Up;
}

// Returns the light level and the location of the brightest light
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const bool sixPoints, const float sixPointsRadius, const bool fourMore, const bool debug)
{
	if (!actor)
		return 0.0f;
	return GetLightingAmount(lightLoc, actor, actor->GetActorLocation(), sixPoints, sixPointsRadius, fourMore, debug);
}
float AMainGameMode::GetLightingAmount(FVector & lightLoc, const FVector location, const bool sixPoints, const float sixPointsRadius, const bool fourMore, const bool debug)
{
	return GetLightingAmount(lightLoc, nullptr, location, sixPoints, sixPointsRadius, fourMore, debug);
}
float AMainGameMode::GetLightingAmount(FVector & lightLoc, const TArray<FVector> locations, const bool debug)
{
	return GetLightingAmount(lightLoc, nullptr, locations, debug);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const FVector location, const bool sixPoints, const float sixPointsRadius, const bool fourMore, const bool debug)
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
	return GetLightingAmount(lightLoc, actor, locations, debug);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const TArray<FVector> locations, const bool debug)
{
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
		if(debug)
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
						
			// If location could be lit
			if (CanSee(actor, location, lightLocation))
			{
				if(debug)
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
// Returns true if one actor/location can see other actor/location
// Its not about visibility to human eye, doesn't take light into account
bool AMainGameMode::CanSee(const AActor * actor1, const AActor * actor2, const bool debug)
{
	if (!actor1 || !actor2)
		return false;

	return CanSee(actor1, actor1->GetActorLocation(), actor2, actor2->GetActorLocation(), debug);
}
bool AMainGameMode::CanSee(const FVector location1, const AActor * actor2, const bool debug)
{
	if (!actor2)
		return false;

	return CanSee(nullptr, location1, actor2, actor2->GetActorLocation(), debug);
}
bool AMainGameMode::CanSee(const AActor * actor1, const FVector location2, const bool debug)
{
	if(!actor1)
		return false;

	return CanSee(actor1, actor1->GetActorLocation(), nullptr, location2, debug);
}
bool AMainGameMode::CanSee(const FVector location1, const FVector location2, const bool debug)
{
	return CanSee(nullptr, location1, nullptr, location2, debug);
}
bool AMainGameMode::CanSee(const AActor * actor1, const FVector location1, const AActor * actor2, const bool debug)
{
	if (!actor2)
		return false;

	return CanSee(actor1, location1, actor2, actor2->GetActorLocation(), debug);
}
bool AMainGameMode::CanSee(const AActor * actor1, const AActor * actor2, const FVector location2, const bool debug)
{
	if (!actor1)
		return false;

	return CanSee(actor1, actor1->GetActorLocation(), actor2, location2, debug);
}
bool AMainGameMode::CanSee(const AActor * actor1, const FVector location1, const FVector location2, const bool debug)
{
	return CanSee(actor1, location1, nullptr, location2, debug);
}
bool AMainGameMode::CanSee(const FVector location1, const AActor * actor2, const FVector location2, const bool debug)
{
	return CanSee(nullptr, location1, actor2, location2, debug);
}
bool AMainGameMode::CanSee(const AActor * actor1, const FVector location1, const AActor * actor2, const FVector location2, const bool debug)
{
	UWorld* gameWorld = GetWorld();
	if (!gameWorld)
		return false;

	FCollisionQueryParams params = FCollisionQueryParams(FName(TEXT("LightTrace")), true);
	if (actor1)
	{
		params.AddIgnoredActor(actor1);
		TInlineComponentArray<UActorComponent*> components;
		actor1->GetComponents(components, true);
		for (UActorComponent* component : components)
			params.AddIgnoredComponent(Cast<UPrimitiveComponent>(component));
	}
	if (actor2)
	{
		params.AddIgnoredActor(actor2);
		TInlineComponentArray<UActorComponent*> components;
		actor2->GetComponents(components, true);
		for (UActorComponent* component : components)
			params.AddIgnoredComponent(Cast<UPrimitiveComponent>(component));
	}

	if (debug)
	{
		DrawDebugPoint(gameWorld, location1, 5, FColor::Red);
		DrawDebugPoint(gameWorld, location2, 5, FColor::Red);
	}

	bool bHit = gameWorld->LineTraceTestByChannel(location1, location2, ECC_Visibility, params);
	// TODO delete if affects performance too much
	// This is a line trace in a different direction helpful in cases when location1 is positioned inside something like a wall which is ignored by the line trace
	if (!bHit)
		bHit = gameWorld->LineTraceTestByChannel(location2, location1, ECC_Visibility, params);

	if (debug && !bHit)
		DrawDebugLine(gameWorld, location1, location2, FColor::Cyan);

	return !bHit;
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
	if (!room)
		return;

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
	AllocatedRoomSpace.Remove(room);
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
void AMainGameMode::PoolMap()
{
	TArray<LabRoom*> rooms;
	SpawnedRoomObjects.GetKeys(rooms);
	for (int i = rooms.Num() - 1; i >= 0; --i)
		PoolRoom(rooms[i]);
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

	if (room)
	{
		if (SpawnedRoomObjects.Contains(room))
			SpawnedRoomObjects[room].Add(lamp);
		if (AllocatedRoomSpace.Contains(room))
			AllocateRoomSpace(room, botLeftX, botLeftY, direction, width, false); 
	}

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

	DeallocateRoom(room);
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

			// Take space inside room so nothing can be spawned there
			AllocateRoomSpace(room, passage->BotLeftX + 1, passage->BotLeftY, MinDistanceInsideToPassage, passage->Width, false);
		}
		// Top wall
		else if (passage->BotLeftY == room->BotLeftY + room->SizeY - 1)
		{
			topWallPositions.Add(passage->BotLeftX - 1 - room->BotLeftX);
			topWallPositions.Add(passage->BotLeftX + passage->Width - room->BotLeftX);

			// Take space inside room so nothing can be spawned there
			AllocateRoomSpace(room, passage->BotLeftX, passage->BotLeftY - MinDistanceInsideToPassage, passage->Width, MinDistanceInsideToPassage, false);
		}
		// Right wall
		else if (passage->BotLeftX == room->BotLeftX + room->SizeX - 1)
		{
			rightWallPositions.Add(passage->BotLeftY - 1 - room->BotLeftY);
			rightWallPositions.Add(passage->BotLeftY + passage->Width - room->BotLeftY);

			// Take space inside room so nothing can be spawned there
			AllocateRoomSpace(room, passage->BotLeftX - MinDistanceInsideToPassage, passage->BotLeftY, MinDistanceInsideToPassage, passage->Width, false);
		}
		// Bottom wall
		else if (passage->BotLeftY == room->BotLeftY)
		{
			bottomWallPositions.Add(passage->BotLeftX - 1 - room->BotLeftX);
			bottomWallPositions.Add(passage->BotLeftX + passage->Width - room->BotLeftX);

			// Take space inside room so nothing can be spawned there
			AllocateRoomSpace(room, passage->BotLeftX, passage->BotLeftY + 1, passage->Width, MinDistanceInsideToPassage, false);
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

//// Space is allocated and can't be allocated again
//FRectSpaceStruct* AMainGameMode::AllocateSpace(LabRoom * room)
//{
//	if (!room)
//		return nullptr;
//
//	FRectSpaceStruct* temp = AllocateSpace(room->BotLeftX, room->BotLeftY, room->SizeX, room->SizeY);
//	if (temp)
//		AllocatedRoomSpace.Add(room, temp);
//
//	return temp;
//}
//FRectSpaceStruct* AMainGameMode::AllocateSpace(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY)
//{
//	FRectSpaceStruct temp = FRectSpaceStruct(botLeftX, botLeftY, sizeX, sizeY);
//	return AllocateSpace(temp);
//}
//FRectSpaceStruct* AMainGameMode::AllocateSpace(FRectSpaceStruct space)
//{
//	if (space.SizeX < 1 || space.SizeY < 1)
//		return nullptr;
//
//	return &AllocatedSpace[AllocatedSpace.Add(space)];
//}
//// Space is not allocated anymore
//void AMainGameMode::DeallocateSpace(FRectSpaceStruct space)
//{
//	AllocatedSpace.Remove(space);
//	// DeallocateSpace(space.BotLeftX, space.BotLeftY, space.SizeX, space.SizeY);
//}
//void AMainGameMode::DeallocateSpace(LabRoom* room)
//{
//	if (!room)
//		return;
//
//	if (AllocatedRoomSpace.Contains(room))
//	{
//		int index;
//		AllocatedSpace.Find(*(AllocatedRoomSpace[room]), index);
//		AllocatedRoomSpace.Remove(room);
//		AllocatedSpace.RemoveAt(index);
//	}
//	else
//		DeallocateSpace(room->BotLeftX, room->BotLeftY, room->SizeX, room->SizeY);
//}
//void AMainGameMode::DeallocateSpace(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY)
//{
//	FRectSpaceStruct* temp = AllocatedSpace.FindByPredicate([botLeftX, botLeftY, sizeX, sizeY](FRectSpaceStruct space)
//	{
//		// ==
//		return space.BotLeftX == botLeftX && space.BotLeftY == botLeftY && space.SizeX == sizeX && space.SizeY == sizeY;
//	});
//	if (!temp)
//		return;
//
//	AllocatedSpace.Remove(*temp);
//}

// Room is allocated and can't be allocated again
void AMainGameMode::AllocateRoom(LabRoom * room)
{
	if (!room)
		return;

	AllocatedRooms.AddUnique(room);
}
// Room is not allocated anymore
void AMainGameMode::DeallocateRoom(LabRoom * room)
{
	if (!room)
		return;

	AllocatedRooms.Remove(room);
}
// Space in the room is allocated and can't be allocated again
void AMainGameMode::AllocateRoomSpace(LabRoom * room, FRectSpaceStruct space, bool local)
{
	AllocateRoomSpace(room, space.BotLeftX, space.BotLeftY, space.SizeX, space.SizeY, local);
}
void AMainGameMode::AllocateRoomSpace(LabRoom * room, const int xOffset, const int yOffset, const EDirectionEnum direction, const int width, bool local)
{
	AllocateRoomSpace(room, xOffset, yOffset, direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down ? width : 1, direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down ? 1 : width, local);
}
void AMainGameMode::AllocateRoomSpace(LabRoom * room, const int xOffset, const int yOffset, const int sizeX, const int sizeY, bool local)
{
	if (local)
		AllocatedRoomSpace[room].Add(FRectSpaceStruct(xOffset, yOffset, sizeX, sizeY));
	else
		AllocatedRoomSpace[room].Add(FRectSpaceStruct(xOffset - room->BotLeftX, yOffset - room->BotLeftY, sizeX, sizeY));
}
// Space in the room is not allocated anymore
void AMainGameMode::DeallocateRoomSpace(LabRoom * room, FRectSpaceStruct space)
{
	// TODO
}

// Returns true if there is free rectangular space
// Returns another room that intersected the sent space
bool AMainGameMode::MapSpaceIsFree(bool amongAllocated, bool amongSpawned, FRectSpaceStruct space)
{
	LabRoom* intersected = nullptr;
	return MapSpaceIsFree(amongAllocated, amongSpawned, space, intersected);
}
bool AMainGameMode::MapSpaceIsFree(bool amongAllocated, bool amongSpawned, const int botLeftX, const int botLeftY, const int sizeX, const int sizeY)
{
	LabRoom* intersected = nullptr;
	return MapSpaceIsFree(amongAllocated, amongSpawned, botLeftX, botLeftY, sizeX, sizeY, intersected);
}
bool AMainGameMode::MapSpaceIsFree(bool amongAllocated, bool amongSpawned, FRectSpaceStruct space, LabRoom*& intersected)
{
	return MapSpaceIsFree(amongAllocated, amongSpawned, space.BotLeftX, space.BotLeftY, space.SizeX, space.SizeY, intersected);
}
bool AMainGameMode::MapSpaceIsFree(bool amongAllocated, bool amongSpawned, const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabRoom*& intersected)
{
	/*if (sizeX < 1 || sizeY < 1)
		return false;*/

	if (amongAllocated)
	{
		bool intersectedAllocated = AllocatedRooms.ContainsByPredicate([botLeftX, botLeftY, sizeX, sizeY, &intersected](LabRoom* room)
		{
			if (!room)
				return false;

			// Not intersecting on X axis
			if (room->BotLeftX + room->SizeX - 1 <= botLeftX)
				return false;
			if (room->BotLeftX >= botLeftX + sizeX - 1)
				return false;

			// Not intersecting on Y axis
			if (room->BotLeftY + room->SizeY - 1 <= botLeftY)
				return false;
			if (room->BotLeftY >= botLeftY + sizeY - 1)
				return false;

			// Intersecting on both axis
			intersected = room;
			return true;
		});
		if (intersectedAllocated)
			return false;
	}
	if (amongSpawned)
	{
		TArray<LabRoom*> spawnedRooms;
		SpawnedRoomObjects.GetKeys(spawnedRooms);
		bool intersectedSpawned = spawnedRooms.ContainsByPredicate([botLeftX, botLeftY, sizeX, sizeY, &intersected](LabRoom* room)
		{
			if (!room)
				return false;

			// Not intersecting on X axis
			if (room->BotLeftX + room->SizeX - 1 <= botLeftX)
				return false;
			if (room->BotLeftX >= botLeftX + sizeX - 1)
				return false;

			// Not intersecting on Y axis
			if (room->BotLeftY + room->SizeY - 1 <= botLeftY)
				return false;
			if (room->BotLeftY >= botLeftY + sizeY - 1)
				return false;

			// Intersecting on both axis
			intersected = room;
			return true;
		});
		if (intersectedSpawned)
			return false;
	}

	return true;
}

// Returns true if there is free rectangular space in a room
// notNearPassage means that space near passages is not free
bool AMainGameMode::RoomSpaceIsFree(LabRoom * room, FRectSpaceStruct space, const bool forPassage, const bool forDoor)
{
	return RoomSpaceIsFree(room, space.BotLeftX, space.BotLeftY, space.SizeX, space.SizeY, forPassage, forDoor);
}
bool AMainGameMode::RoomSpaceIsFree(LabRoom * room, const int xOffset, const int yOffset, EDirectionEnum direction, const int width, const bool forPassage, const bool forDoor)
{
	return RoomSpaceIsFree(room, xOffset, yOffset, direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down ? width : 1, direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down ? 1 : width, forPassage, forDoor);
}
bool AMainGameMode::RoomSpaceIsFree(LabRoom * room, const int xOffset, const int yOffset, const int sizeX, const int sizeY, const bool forPassage, const bool forDoor)
{
	if (!room)
		return false;

	if (forPassage)
	{
		if (xOffset < 0 || yOffset < 0 || sizeX < 1 || sizeY < 1 || xOffset + sizeX > room->SizeX || yOffset + sizeY > room->SizeY)
			return false;
		// Space is withing room borders

		// Determining what wall is passage on
		EDirectionEnum wallDirection;
		if (xOffset == 0 && sizeX == 1)
			wallDirection = EDirectionEnum::VE_Left;
		else if (xOffset == room->SizeX - 1)
			wallDirection = EDirectionEnum::VE_Right;
		else if (yOffset == 0 && sizeY == 1)
			wallDirection = EDirectionEnum::VE_Down;
		else if (yOffset == room->SizeY - 1)
			wallDirection = EDirectionEnum::VE_Up;
		else
			return false;

		if (wallDirection == EDirectionEnum::VE_Left || wallDirection == EDirectionEnum::VE_Right)
		{
			int extra = !forDoor ? MinDistanceBetweenPassages : FMath::Max(MinDistanceBetweenPassages, sizeY / 2 + sizeY % 2);
			if (yOffset < extra || yOffset + sizeY > room->SizeY - extra)
				return false;

			bool intersected = room->Passages.ContainsByPredicate([extra, wallDirection, room, yOffset, sizeY](LabPassage* passage)
			{
				if (!passage)
					return false;

				// Different wall
				if ((wallDirection == EDirectionEnum::VE_Left && passage->BotLeftX != room->BotLeftX) || (wallDirection == EDirectionEnum::VE_Right && passage->BotLeftX != room->BotLeftX + room->SizeX - 1))
					return false;

				int localExtra = !passage->bIsDoor ? extra : FMath::Max(extra, passage->Width / 2 + passage->Width % 2);

				// Not intersecting on Y axis
				if (passage->BotLeftY - room->BotLeftY + passage->Width - 1 + localExtra < yOffset)
					return false;
				if (passage->BotLeftY - room->BotLeftY > yOffset + sizeY - 1 + localExtra)
					return false;

				// Intersecting
				return true;
			});
			return !intersected;
		}
		else
		{
			int extra = !forDoor ? MinDistanceBetweenPassages : FMath::Max(MinDistanceBetweenPassages, sizeX / 2 + sizeX % 2);
			if (xOffset < extra || xOffset + sizeX > room->SizeX - extra)
				return false;

			bool intersected = room->Passages.ContainsByPredicate([extra, wallDirection, room, xOffset, sizeX](LabPassage* passage)
			{
				if (!passage)
					return false;

				// Different wall
				if ((wallDirection == EDirectionEnum::VE_Down && passage->BotLeftY != room->BotLeftY) || (wallDirection == EDirectionEnum::VE_Up && passage->BotLeftY != room->BotLeftY + room->SizeY - 1))
					return false;

				int localExtra = !passage->bIsDoor ? extra : FMath::Max(extra, passage->Width / 2 + passage->Width % 2);

				// Not intersecting on X axis
				if (passage->BotLeftX - room->BotLeftX + passage->Width - 1 + localExtra < xOffset)
					return false;
				if (passage->BotLeftX - room->BotLeftX > xOffset + sizeX - 1 + localExtra)
					return false;

				// Intersecting
				return true;
			});
			return !intersected;
		}
	}
	else
	{
		if (xOffset < 1 || yOffset < 1 || sizeX < 1 || sizeY < 1 || xOffset + sizeX > room->SizeX - 1 || yOffset + sizeY > room->SizeY - 1)
			return false;
		// Space is withing room borders including walls

		bool intersected = AllocatedRoomSpace[room].ContainsByPredicate([xOffset, yOffset, sizeX, sizeY](FRectSpaceStruct space)
		{
			// Not intersecting on X axis
			if (space.BotLeftX + space.SizeX - 1 < xOffset)
				return false;
			if (space.BotLeftX > xOffset + sizeX - 1)
				return false;

			// Not intersecting on Y axis
			if (space.BotLeftY + space.SizeY - 1 < yOffset)
				return false;
			if (space.BotLeftY > yOffset + sizeY - 1)
				return false;

			// Intersecting on both axis
			return true;
		});

		return !intersected;
	}
}

// Returns true is first is inside second
bool AMainGameMode::IsInside(LabRoom * room1, LabRoom * room2)
{
	if (!room1 || !room2)
		return false;
}
bool AMainGameMode::IsInside(FRectSpaceStruct space1, LabRoom * room2)
{
	if (!room2)
		return false;

	return IsInside(space1, FRectSpaceStruct(room2->BotLeftX, room2->BotLeftY, room2->SizeX, room2->SizeY));
}
bool AMainGameMode::IsInside(LabRoom * room1, FRectSpaceStruct space2)
{
	if (!room1)
		return false;

	return IsInside(FRectSpaceStruct(room1->BotLeftX, room1->BotLeftY, room1->SizeX, room1->SizeY), space2);
}
bool AMainGameMode::IsInside(FRectSpaceStruct space1, FRectSpaceStruct space2)
{
	// Out on the left
	if(space1.BotLeftX < space2.BotLeftX)
		return false;

	// Out on the bottom
	if (space1.BotLeftY < space2.BotLeftY)
		return false;

	// Out on the right
	if (space1.BotLeftX + space1.SizeX > space2.BotLeftX + space2.SizeX)
		return false;

	// Out on the top
	if (space1.BotLeftY + space1.SizeY > space2.BotLeftY + space2.SizeY)
		return false;
}

// Tries to create a room and allocate space for it
LabRoom* AMainGameMode::CreateRoom(FRectSpaceStruct space)
{
	return CreateRoom(space.BotLeftX, space.BotLeftY, space.SizeX, space.SizeY);
}
LabRoom* AMainGameMode::CreateRoom(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY)
{
	/*if(sizeX < 4 || sizeY < 4 || !MapSpaceIsFree(true, true, botLeftX, botLeftY, sizeX, sizeY))
		return nullptr;*/

	LabRoom* room = new LabRoom(botLeftX, botLeftY, sizeX, sizeY);
	AllocateRoom(room);
	AllocatedRoomSpace.Add(room);

	return room;
}
// Creates starting room
LabRoom * AMainGameMode::CreateStartRoom()
{
	// TODO should be random
	return CreateRoom(-10, -5, 15, 12);;
}

// Creates random space for a future passage (not world location but offsets)
// Doesn't take other passages into account. Direction is always out
FRectSpaceStruct AMainGameMode::CreateRandomPassageSpace(LabRoom * room, EDirectionEnum& direction, const bool forDoor)
{
	FRectSpaceStruct space;

	// Choose wall
	direction = RandDirection();
	switch (direction)
	{
	case EDirectionEnum::VE_Left:
		space.BotLeftX = 0;
		break;
	case EDirectionEnum::VE_Right:
		space.BotLeftX = room->SizeX - 1;
		break;
	case EDirectionEnum::VE_Down:
		space.BotLeftY = 0;
		break;
	case EDirectionEnum::VE_Up:
		space.BotLeftY = room->SizeY - 1;
		break;
	}

	int doorWidth = !forDoor ? 0 : RandBool(DoorIsNormalProbability) ? NormalDoorWidth : BigDoorWidth;
	int minPos = !forDoor ? MinDistanceBetweenPassages : FMath::Max(MinDistanceBetweenPassages, doorWidth / 2 + doorWidth % 2);

	// Left or right
	if (direction == EDirectionEnum::VE_Left || direction == EDirectionEnum::VE_Right)
	{
		space.SizeX = 1;

		if (forDoor)
			space.SizeY = doorWidth;
		else
			space.SizeY = FMath::RandRange(MinPassageWidth, FMath::Min(MaxPassageWidth, room->SizeY - 2 * MinDistanceBetweenPassages));

		int maxYpos = room->SizeY - 1 - minPos - space.SizeY;
		space.BotLeftY = FMath::RandRange(minPos, maxYpos);
	}
	// Bottom or top
	else
	{
		space.SizeY = 1;

		if (forDoor)
			space.SizeX = doorWidth;
		else
			space.SizeX = FMath::RandRange(MinPassageWidth, FMath::Min(MaxPassageWidth, room->SizeX - 2 * MinDistanceBetweenPassages));

		int maxXpos = room->SizeX - 1 - minPos - space.SizeX;
		space.BotLeftX = FMath::RandRange(minPos, maxXpos);
	}

	return space;
}

// Creates minimum space for a room near passage for tests and allocation
// TODO maybe it should take room size just in case other room gets destroyed
FRectSpaceStruct AMainGameMode::CreateMinimumRoomSpace(LabRoom* room, FRectSpaceStruct passageSpace, EDirectionEnum direction)
{
	FRectSpaceStruct space;

	// int delta = !forDoor ? MinDistanceBetweenPassages : FMath::Max(MinDistanceBetweenPassages, NormalDoorWidth / 2 + NormalDoorWidth % 2);
	int delta = MinDistanceBetweenPassages;

	switch (direction)
	{
	case EDirectionEnum::VE_Left:
		space.BotLeftX = room->BotLeftX - MinRoomSize + 1;
		break;
	case EDirectionEnum::VE_Right:
		space.BotLeftX = room->BotLeftX + room->SizeX - 1;
	case EDirectionEnum::VE_Down:
		space.BotLeftY = room->BotLeftY - MinRoomSize + 1;
		break;
	case EDirectionEnum::VE_Up:
		space.BotLeftY = room->BotLeftY + room->SizeY - 1;
	}

	// Left or right
	if (direction == EDirectionEnum::VE_Left || direction == EDirectionEnum::VE_Right)
	{
		space.BotLeftY = room->BotLeftY + passageSpace.BotLeftY - delta;
		space.SizeX = MinRoomSize;
		space.SizeY = passageSpace.SizeY + 2 * delta;
	}
	// Bottom or top
	else
	{
		space.BotLeftX = room->BotLeftX + passageSpace.BotLeftX - delta;
		space.SizeX = passageSpace.SizeX + 2 * delta;
		space.SizeY = MinRoomSize;
	}

	return space;
}
// TODO CreateRandomRoomSpace

// Creates and adds a random passage to the room, returns passage or nullptr, also allocates room space and returns allocated room space by reference
LabPassage * AMainGameMode::CreateAndAddRandomPassage(LabRoom * room, FRectSpaceStruct & roomSpace, LabRoom*& possibleRoomConnection)
{
	bool forDoor = RandBool(PassageIsDoorProbability);
	EDirectionEnum direction;

	// Find random position for the new passage
	FRectSpaceStruct pasSpace = CreateRandomPassageSpace(room, direction, forDoor);
	// Test if it works in the room
	if (!RoomSpaceIsFree(room, pasSpace, true, forDoor))
		return nullptr;

	// Find minimum space for a new room on the other side of this passage
	roomSpace = CreateMinimumRoomSpace(room, pasSpace, direction);

	// Test if it works on the map
	// Intersects something spawned
	if (!MapSpaceIsFree(false, true, roomSpace)) 
		return nullptr;

	LabRoom* intersected = nullptr;
	// Intersects something allocated
	bool spaceIsFree = MapSpaceIsFree(true, false, roomSpace, intersected); 

	// TODO somewhere we find possibleRoomConnection
	// Make sure that not all intersects are possible connections
	// Includes minimum completely
	if (!spaceIsFree)
		possibleRoomConnection = nullptr; // TODO nullptr obviously

	// If we have something to connect to but we don't want to
	if (possibleRoomConnection && !RandBool(ConnectToOtherRoomProbability))
		return nullptr;

	// Add this passage to the room
	LabPassage* passage;
	if (!forDoor)
		passage = room->AddPassage(room->BotLeftX + pasSpace.BotLeftX, room->BotLeftY + pasSpace.BotLeftY, direction, possibleRoomConnection, direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down ? pasSpace.SizeX : pasSpace.SizeY);
	else
	{
		// TODO maybe it shouldn't allow every color
		FLinearColor color = RandColor();

		passage = room->AddPassage(room->BotLeftX + pasSpace.BotLeftX, room->BotLeftY + pasSpace.BotLeftY, direction, possibleRoomConnection, forDoor, color, direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down ? pasSpace.SizeX : pasSpace.SizeY);
	}

	return passage;
}

// Creates new passages in the room
// Create new rooms for passages 
// Returns new rooms
TArray<LabRoom*> AMainGameMode::ExpandRoom(LabRoom * room)
{
	TArray<LabRoom*> newRooms;

	// The number of passages we want to have in the room
	// These are not just new but overall
	int desiredNumOfPassages = FMath::RandRange(MinRoomNumOfPassages, MaxRoomNumOfPassages);
	UE_LOG(LogTemp, Warning, TEXT("Trying to add %d passages"), desiredNumOfPassages);

	// Maximum number of tries
	int maxTries = MaxRoomPassageCreationTriesPerDesired * desiredNumOfPassages;

	// Creates new passages in the room
	// Allocates minimum room space for passages
	// Create rooms for the passages if rooms weren't found already
	for (int i = 0; room->Passages.Num() < MinRoomNumOfPassages || (i < maxTries && room->Passages.Num() < desiredNumOfPassages); ++i)
	{
		FRectSpaceStruct minRoomSpace;
		LabRoom* possibleRoomConnection = nullptr;
		LabPassage* passage = CreateAndAddRandomPassage(room, minRoomSpace, possibleRoomConnection);
		if (passage)
		{
			if (!possibleRoomConnection)
			{
				UE_LOG(LogTemp, Warning, TEXT("> %s"), TEXT("success"));

				UE_LOG(LogTemp, Warning, TEXT("> x: %d, y: %d, sX: %d, sY: %d"), minRoomSpace.BotLeftX, minRoomSpace.BotLeftY, minRoomSpace.SizeX, minRoomSpace.SizeY);

				// TODO create new room space 
				// CreateRandomRoomSpace()
				// Make sure that MapSpaceIsFree(newSpace)
				FRectSpaceStruct newSpace = minRoomSpace;

				// We create new room from new space, it also allocates room's space
				LabRoom* newRoom = CreateRoom(newSpace);
				if (!newRoom)
					continue;

				// We add passage to the room
				newRoom->AddPassage(passage);
				newRooms.Add(newRoom);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("> %s"), TEXT("found a room to connect to"));

				// Should add passage to the found room if we found one
				// (unless it was added before)
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("> %s"), TEXT("failure"));
		}
	}
	// TODO We should also find passages between this room and other allocated but not spawn

	return newRooms;
}

// Creates random space in the room with specified size for a future object in the room (not world location but offset)
// Returns false if couldn't create
bool AMainGameMode::CreateRandomInsideSpaceOfSize(LabRoom * room, int& xOffset, int& yOffset, const int sizeX, const int sizeY, const bool canBeTaken)
{
	if (sizeX < 1 || sizeY < 1 || sizeX > room->SizeX - 2 || sizeY > room->SizeY - 2)
		return false;

	xOffset = FMath::RandRange(1, room->SizeX - 1 - sizeX);
	yOffset = FMath::RandRange(1, room->SizeY - 1 - sizeY);

	// Test if it works in the room
	return canBeTaken || RoomSpaceIsFree(room, xOffset, yOffset, sizeX, sizeY);
}
// Same but near wall and returns direction from wall (width is along wall)
bool AMainGameMode::CreateRandomInsideSpaceOfWidthNearWall(LabRoom * room, int& xOffset, int& yOffset, const int width, EDirectionEnum & direction, const bool canBeTaken)
{
	// Choose wall
	direction = RandDirection(); // Direction here are INTO room, so wall is the opposite
	switch (direction)
	{
	case EDirectionEnum::VE_Right: // Left wall
		xOffset = 1;
		break;
	case EDirectionEnum::VE_Left: // Right wall
		xOffset = room->SizeX - 2;
		break;
	case EDirectionEnum::VE_Up: // Bottom wall
		yOffset = 1;
		break;
	case EDirectionEnum::VE_Down: // Top wall
		yOffset = room->SizeY - 2;
		break;
	}

	// Left or right
	if (direction == EDirectionEnum::VE_Left || direction == EDirectionEnum::VE_Right) 
		yOffset = FMath::RandRange(1, room->SizeY - 1 - width);
	// Bottom or top
	else 
		xOffset = FMath::RandRange(1, room->SizeX - 1 - width);

	// Test if it works in the room
	return canBeTaken || RoomSpaceIsFree(room, xOffset, yOffset, direction, width);
}

// Fills room with random objects, spawns and returns them
// Should always be called on a room that is already spawned
TArray<AActor*> AMainGameMode::FillRoom(LabRoom* room, int minNumOfLampsOverride)
{
	TArray<AActor*> spawnedActors;

	// The number of lamps we want to have in the room
	int desiredNumOfLamps = FMath::RandRange(MinRoomNumOfLamps, 1 + MaxRoomNumOfLampsPerHundredArea * room->SizeX * room->SizeY / 100);

	// Maximum number of tries
	int maxTries = MaxRoomLampCreationTriesPerDesired * desiredNumOfLamps;

	// Creates new lamps in the room
	for (int i = 0; spawnedActors.Num() < MinRoomNumOfLamps || (i < maxTries && spawnedActors.Num() < desiredNumOfLamps) || spawnedActors.Num() < minNumOfLampsOverride; ++i)
	{
		int xOff;
		int yOff;
		int width = FMath::RandRange(MinLampWidth, MaxLampWidth); 
		EDirectionEnum direction;
		if (CreateRandomInsideSpaceOfWidthNearWall(room, xOff, yOff, width, direction))
		{
			// TODO color should depend on the room
			FLinearColor color = RandColor(); 
			AWallLamp* lamp = SpawnWallLamp(room->BotLeftX + xOff, room->BotLeftY + yOff, direction, color, width, room);
			// TODO this shouldn't be here
			if (lamp) lamp->Execute_ActivateIndirectly(lamp); 
			spawnedActors.Add(lamp);
		}
	}

	// Creates a flashlight
	bool shouldSpawnFlashlight = RandBool(SpawnFlashlightProbability);
	for (int i = 0; shouldSpawnFlashlight && i < MaxGenericSpawnTries; ++i)
	{
		int xOff;
		int yOff;
		if (CreateRandomInsideSpaceOfSize(room, xOff, yOff, 1, 1, true))
		{
			EDirectionEnum direction = RandDirection();
			AFlashlight* flashlight = SpawnFlashlight(room->BotLeftX + xOff, room->BotLeftY + yOff, direction);
			spawnedActors.Add(flashlight);
			break; // We only spawn once
		}
	}

	return spawnedActors;
}

// Generates map
void AMainGameMode::GenerateMap()
{
	// Generation:

	// 1. Something allocates a room
	LabRoom* startRoom = CreateStartRoom();
	LabRoom* leftRoom = CreateRoom(-17, -5, 5, 12);

	// 2. We find positions for passages and allocate minimum size
	// 2*. We should also find passages between this room and other allocated but not spawn 
	// 3. For each passage we find maximum possible distance in three directions
	// 4. For each passage we allocate new rooms (1. may be happening here)
	// 5. We add passages to our new rooms
	TArray<LabRoom*> newRooms = ExpandRoom(startRoom);
	TArray<LabRoom*> newLeftRooms = ExpandRoom(leftRoom); // THIS should produce passages to the rooms from start room

	// 6. We spawn our initial room
	SpawnRoom(startRoom);
	SpawnRoom(leftRoom);

	// 7. We initialize and spawn other parts of the initial room
	FillRoom(startRoom, 1); // we want at least one lamp in starting room so we override it
	FillRoom(leftRoom);

	// 8+. We repeat from 1 for other rooms (rn we just spawn)
	for (LabRoom* room : newRooms)
	{
		// TArray<LabRoom*> evenNewerRooms = ExpandRoom(room)
		SpawnRoom(room);
		FillRoom(room);

		// Repeat
	}
	for (LabRoom* room : newLeftRooms)
	{
		// TArray<LabRoom*> evenNewerRooms = ExpandRoom(room)
		SpawnRoom(room);
		FillRoom(room);

		// Repeat
	}
}
// Resets the map
void AMainGameMode::ResetMap()
{
	PoolMap();
	GenerateMap();
}

// Shows/hides debug
void AMainGameMode::ShowHideDebug()
{
	bShowDebug = !bShowDebug;
	if (DarknessController)
		DarknessController->SetShowDebug(bShowDebug);
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

	// Set to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	// We find darkness controller
	UWorld* gameWorld = GetWorld();
	for (TObjectIterator<ADarknessController> Itr; Itr; ++Itr)
	{
		// World Check
		if (Itr->GetWorld() != gameWorld)
			continue;

		DarknessController = *Itr;
		break; // We only need one (and there should be only one)
	}

	GenerateMap();
	/*PoolMap();
	GenerateMap();*/
	
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
	*/
}

// Called every frame
void AMainGameMode::Tick(const float deltaTime)
{
	Super::Tick(deltaTime);

	// TODO delete, used just for test
	/*TimeSinceLastGeneration += deltaTime;
	if (TimeSinceLastGeneration > 0.1f)
	{
		TimeSinceLastGeneration = 0.0f;
		PoolMap();
		GenerateMap();
	}*/
}

// Called when actor is being removed from the play
void AMainGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UE_LOG(LogTemp, Warning, TEXT("EndPlay called"));

	TArray<LabRoom*> allRooms;
	AllocatedRoomSpace.GetKeys(allRooms);

	// Clear all saved rooms
	for (int i = allRooms.Num() - 1; i >= 0; --i)
		delete allRooms[i];
}

// TODO delete?
// Called at start of seamless travel, or right before map change for hard travel
void AMainGameMode::StartToLeaveMap()
{
	Super::StartToLeaveMap();

	UE_LOG(LogTemp, Warning, TEXT("StartToLeaveMap called"));
}