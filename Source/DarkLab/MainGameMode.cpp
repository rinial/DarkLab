// Fill out your copyright notice in the Description page of Project Settings.

#include "MainGameMode.h"
#include "EngineUtils.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "UObject/UObjectIterator.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"
#include "BasicWall.h"
#include "BasicDoor.h"
#include "Flashlight.h"

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

		pointLights.Add(*Itr);
	}

	// We find local results for all locations
	for (FVector location : locations)
	{
		// DrawDebugPoint(gameWorld, location, 5, FColor::Red);

		// This will be used for the spot lights
		FBoxSphereBounds bounds = FBoxSphereBounds(location, FVector(1, 1, 1), 1);

		// We take the highest local result among lights
		for (UPointLightComponent* lightComp : pointLights)
		{
			// We don't care about invisible lights
			if (!lightComp->IsVisible())
				continue;

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

			// If location could be lit
			if (!bHit)
			{
				// DrawDebugLine(gameWorld, location, lightLocation, FColor::Cyan);

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

// Spawn specific objects
void AMainGameMode::SpawnBasicWall(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY)
{
	ABasicWall* wall;
	if (BasicWallPool.Num() > 0)
	{
		wall = BasicWallPool[0];
		BasicWallPool.RemoveAt(0);
	}
	else
		wall = GetWorld()->SpawnActor<ABasicWall>(BasicWallBP);
	if (!wall)
		return;

	PlaceObject(wall, botLeftX, botLeftY, EDirectionEnum::VE_Up, true, sizeX, sizeY);
	wall->Execute_SetActive(wall, true);

	UE_LOG(LogTemp, Warning, TEXT("Spawned a basic wall"));
}
void AMainGameMode::SpawnBasicDoor(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color)
{
	ABasicDoor* door;
	if (BasicDoorPool.Num() > 0)
	{
		door = BasicDoorPool[0];
		BasicDoorPool.RemoveAt(0);
	}
	else
		door = GetWorld()->SpawnActor<ABasicDoor>(BasicDoorBP);
	if (!door)
		return;

	PlaceObject(door, botLeftX, botLeftY, direction);
	door->DoorColor = color;
	door->Execute_SetActive(door, true);

	UE_LOG(LogTemp, Warning, TEXT("Spawned a basic door"));
}
void AMainGameMode::SpawnFlashlight(const int botLeftX, const int botLeftY, const EDirectionEnum direction)
{
	AFlashlight* flashlight = GetWorld()->SpawnActor<AFlashlight>(FlashlightBP);
	if (!flashlight)
		return;

	PlaceObject(flashlight, botLeftX, botLeftY, direction);

	UE_LOG(LogTemp, Warning, TEXT("Spawned a flashlight"));
}

// Sets default values
AMainGameMode::AMainGameMode()
{
	// Find blueprints and save found class for future spawns
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

	SpawnBasicWall(-7, -5, 1, 10);
	SpawnBasicDoor(-6, 4, EDirectionEnum::VE_Up, FLinearColor::Red);
	SpawnFlashlight(0, 0);
}