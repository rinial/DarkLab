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
#include "Flashlight.h"
#include "LabPassage.h"
#include "LabRoom.h"
#include "LabHallway.h"

// Returns the light level and the location of the brightest light
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const bool sixPoints, const float sixPointsRadius)
{
	if (!actor)
		return 0.0f;
	return GetLightingAmount(lightLoc, actor, actor->GetActorLocation(), sixPoints, sixPointsRadius);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const FVector location, const bool sixPoints, const float sixPointsRadius)
{
	return GetLightingAmount(lightLoc, nullptr, location, sixPoints, sixPointsRadius);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const TArray<FVector> locations)
{
	return GetLightingAmount(lightLoc, nullptr, locations);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const FVector location, const bool sixPoints, const float sixPointsRadius)
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

// Places an object on the map (size < 1 means that we take current size for this axis)
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftLocX, const int botLeftLocY, const EDirectionEnum direction, const bool setSizeFirst, const int sizeX, const int sizeY, const int sizeZ)
{
	PlaceObject(object, botLeftLocX, botLeftLocY, 0, direction, setSizeFirst, sizeX, sizeY, sizeZ);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftLocX, const int botLeftLocY, const int botLeftLocZ, const EDirectionEnum direction, const bool setSizeFirst, const int sizeX, const int sizeY, const int sizeZ)
{
	PlaceObject(object, FIntVector(botLeftLocX, botLeftLocY, botLeftLocZ), direction, setSizeFirst, sizeX, sizeY, sizeZ);
}
void AMainGameMode::PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc, const EDirectionEnum direction, const bool setSizeFirst, const int sizeX, const int sizeY, const int sizeZ)
{
	UObject* obj = object->_getUObject();

	if (setSizeFirst)
	{
		if (sizeZ < 1)
			object->Execute_SetSizeXY(obj, sizeX, sizeY);
		else
			object->Execute_SetSize(obj, FIntVector(sizeX, sizeY, sizeZ));
	}

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
ABasicFloor * AMainGameMode::SpawnBasicFloor(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY)
{
	ABasicFloor* floor = Cast<ABasicFloor>(TryGetPoolable(BasicFloorBP));
	if (!floor)
		floor = GetWorld()->SpawnActor<ABasicFloor>(BasicFloorBP);

	PlaceObject(floor, botLeftX, botLeftY, EDirectionEnum::VE_Up, true, sizeX, sizeY);

	UE_LOG(LogTemp, Warning, TEXT("Spawned a basic floor"));

	return floor;
}
ABasicWall* AMainGameMode::SpawnBasicWall(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY)
{
	ABasicWall* wall = Cast<ABasicWall>(TryGetPoolable(BasicWallBP));
	if (!wall)
		wall = GetWorld()->SpawnActor<ABasicWall>(BasicWallBP);

	PlaceObject(wall, botLeftX, botLeftY, EDirectionEnum::VE_Up, true, sizeX, sizeY);

	UE_LOG(LogTemp, Warning, TEXT("Spawned a basic wall"));

	return wall;
}
ABasicDoor * AMainGameMode::SpawnBasicDoor(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color, const int width)
{
	ABasicDoor* door = Cast<ABasicDoor>(TryGetPoolable(BasicDoorBP));
	if (!door)
		door = GetWorld()->SpawnActor<ABasicDoor>(BasicDoorBP);

	door->DoorColor = color;
	PlaceObject(door, botLeftX, botLeftY, direction, true, width);

	UE_LOG(LogTemp, Warning, TEXT("Spawned a basic door"));

	return door;
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
TArray<TScriptInterface<IDeactivatable>> AMainGameMode::SpawnRoom(LabRoom * room)
{
	TArray<TScriptInterface<IDeactivatable>> spawned;

	// Spawning floor
	if(!room->bIsInner)
		spawned.Add(SpawnBasicFloor(room->BotLeftLocX, room->BotLeftLocY, room->SizeX, room->SizeY));

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
		if (passage->BotLeftLocX == room->BotLeftLocX)
		{
			leftWallPositions.Add(passage->BotLeftLocY - 1 - room->BotLeftLocY);
			leftWallPositions.Add(passage->BotLeftLocY + passage->Width - room->BotLeftLocY);
		}
		// Top wall
		else if (passage->BotLeftLocY == room->BotLeftLocY + room->SizeY - 1)
		{
			topWallPositions.Add(passage->BotLeftLocX - 1 - room->BotLeftLocX);
			topWallPositions.Add(passage->BotLeftLocX + passage->Width - room->BotLeftLocX);
		}
		// Right wall
		else if (passage->BotLeftLocX == room->BotLeftLocX + room->SizeX - 1)
		{
			rightWallPositions.Add(passage->BotLeftLocY - 1 - room->BotLeftLocY);
			rightWallPositions.Add(passage->BotLeftLocY + passage->Width - room->BotLeftLocY);
		}
		// Bottom wall
		else if (passage->BotLeftLocY == room->BotLeftLocY)
		{
			bottomWallPositions.Add(passage->BotLeftLocX - 1 - room->BotLeftLocX);
			bottomWallPositions.Add(passage->BotLeftLocX + passage->Width - room->BotLeftLocX);
		}

		// Spawn door if needed
		if (passage->bIsDoor)
			spawned.Add(SpawnBasicDoor(passage->BotLeftLocX, passage->BotLeftLocY, passage->GridDirection, passage->Color, passage->Width));
	}

	leftWallPositions.Sort();
	topWallPositions.Sort();
	rightWallPositions.Sort();
	bottomWallPositions.Sort();

	// Spawning left walls
	for (int i = 0; i + 1 < leftWallPositions.Num(); i += 2)
	{
		int wallLength = leftWallPositions[i + 1] - leftWallPositions[i] + 1;
		spawned.Add(SpawnBasicWall(room->BotLeftLocX, room->BotLeftLocY + leftWallPositions[i], 1, wallLength));
	}
	// Spawning top walls
	for (int i = 0; i + 1 < topWallPositions.Num(); i += 2)
	{
		int wallLength = topWallPositions[i + 1] - topWallPositions[i] + 1;
		spawned.Add(SpawnBasicWall(room->BotLeftLocX + topWallPositions[i], room->BotLeftLocY + room->SizeY - 1, wallLength, 1));
	}
	// Spawning right walls
	for (int i = 0; i + 1 < rightWallPositions.Num(); i += 2)
	{
		int wallLength = rightWallPositions[i + 1] - rightWallPositions[i] + 1;
		spawned.Add(SpawnBasicWall(room->BotLeftLocX + room->SizeX - 1, room->BotLeftLocY + rightWallPositions[i], 1, wallLength));
	}
	// Spawning bottom walls
	for (int i = 0; i + 1 < bottomWallPositions.Num(); i += 2)
	{
		int wallLength = bottomWallPositions[i + 1] - bottomWallPositions[i] + 1;
		spawned.Add(SpawnBasicWall(room->BotLeftLocX + bottomWallPositions[i], room->BotLeftLocY, wallLength, 1));
	}

	return spawned;
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
	static ConstructorHelpers::FObjectFinder<UClass> flashlightBP(TEXT("Class'/Game/Blueprints/FlashlightBP.FlashlightBP_C'"));
	if (flashlightBP.Succeeded())
		FlashlightBP = flashlightBP.Object;
}

// Called when the game starts or when spawned
void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	// TODO shouldn't create and spawn rooms from here

	LabRoom* room1 = new LabRoom(-50, -50, 100, 100);
	room1->AddPassage(9, -50, EDirectionEnum::VE_Down, nullptr, true);

	LabRoom* room2 = new LabRoom(-10, -5, 15, 12, true);
	room2->AddPassage(-10, -3, EDirectionEnum::VE_Left, nullptr, true);
	room2->AddPassage(4, 1, EDirectionEnum::VE_Left, nullptr, true);
	room2->AddPassage(-6, 6, EDirectionEnum::VE_Up, nullptr, true, FLinearColor::Red);
	room2->AddPassage(0, -5, EDirectionEnum::VE_Up, nullptr, 2);

	LabHallway* hallway1 = new LabHallway(4, -4, EDirectionEnum::VE_Right, 46, 4, room2, room1, 2, 2, true);

	LabHallway* hallway2 = new LabHallway(-5, -25, EDirectionEnum::VE_Right, 50, 12, nullptr, nullptr, false, true, FLinearColor::White, FLinearColor::Black, 10, 6, true);

	// Testing pooling
	PoolObject(SpawnBasicWall(2, 3, 5, 1));

	TArray<TScriptInterface<IDeactivatable>> temp = SpawnRoom(room1);
	PoolObjects(temp);

	// Spawning stuff
	SpawnRoom(room1);
	SpawnRoom(room2);
	SpawnRoom(hallway1);
	SpawnRoom(hallway2);

	SpawnFlashlight(0, 0);
	
	// SpawnBasicFloor(-20, -20, 40, 40);
	// SpawnBasicWall(-7, -5, 1, 9);
	// SpawnBasicDoor(-6, 3, EDirectionEnum::VE_Up, FLinearColor::Red);

	delete room1;
	delete room2;
	delete hallway1;
	delete hallway2;
}