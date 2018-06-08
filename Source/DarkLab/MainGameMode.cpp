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
#include "Lighter.h"
#include "Doorcard.h"
#include "ExitVolume.h"
#include "LabPassage.h"
#include "LabRoom.h"
// #include "LabHallway.h"
#include "DarknessController.h"
#include "Darkness.h"
#include "MainPlayerController.h"
#include "MainCharacter.h"
#include "GameHUD.h"
// For on screen debug
#include "EngineGlobals.h"
#include "Engine/Engine.h"

// Probabilities
const float AMainGameMode::ReshapeDarknessOnEnterProbability = 0.7f;
const float AMainGameMode::ReshapeDarknessOnTickProbability = 0.6f;
const float AMainGameMode::LampsTurnOnOnEnterProbability = 0.6f;
const float AMainGameMode::LampsTurnOffPerSecondProbability = 0.04f;
const float AMainGameMode::AllLampsInRoomTurnOffProbability = 0.15f;
const float AMainGameMode::ConnectToOtherRoomProbability = 0.8f;
const float AMainGameMode::DeletePassageToFixProbability = 0.0f; // TODO increase or delete?
const float AMainGameMode::PassageIsDoorProbability = 0.45f;
const float AMainGameMode::DoorIsNormalProbability = 1.f; // 0.90f; TODO decrease if we need some big doors
const float AMainGameMode::DoorIsExitProbability = 0.17f;
const float AMainGameMode::SpawnFlashlightProbability = 0.17f; // TODO decrease
const float AMainGameMode::SpawnDoorcardProbability = 0.28f;
const float AMainGameMode::MakeRoomSpecialForCardProbability = 0.0f; // TODO increase or delete?
const float AMainGameMode::BlueProbability = 0.14f;
const float AMainGameMode::GreenProbability = 0.12f;
const float AMainGameMode::YellowProbability = 0.10f;
const float AMainGameMode::RedProbability = 0.08f;
const float AMainGameMode::BlackProbability = 0.06f;
// Other constants
const float AMainGameMode::ReshapeDarknessTick = 4.f;

// Returns true with certain probability
bool AMainGameMode::RandBool(const float probability)
{
	float temp = FMath::FRand();
	temp = temp >= 1.f ? 0.f : temp;
	return temp < probability;
}
// Returns random color with certain probabilities
FLinearColor AMainGameMode::RandColor()
{
	// TODO make colors constants somewhere

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
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const bool sixPoints, const float sixPointsRadius, const bool fourMore, const bool returnFirstPositive)
{
	if (!actor)
		return 0.0f;
	return GetLightingAmount(lightLoc, actor, actor->GetActorLocation(), sixPoints, sixPointsRadius, fourMore, returnFirstPositive);
}
float AMainGameMode::GetLightingAmount(FVector & lightLoc, const FVector location, const bool sixPoints, const float sixPointsRadius, const bool fourMore, const bool returnFirstPositive)
{
	return GetLightingAmount(lightLoc, nullptr, location, sixPoints, sixPointsRadius, fourMore, returnFirstPositive);
}
float AMainGameMode::GetLightingAmount(FVector & lightLoc, const TArray<FVector> locations, const bool returnFirstPositive)
{
	return GetLightingAmount(lightLoc, nullptr, locations, returnFirstPositive);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const FVector location, const bool sixPoints, const float sixPointsRadius, const bool fourMore, const bool returnFirstPositive)
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
	return GetLightingAmount(lightLoc, actor, locations, returnFirstPositive);
}
float AMainGameMode::GetLightingAmount(FVector& lightLoc, const AActor* actor, const TArray<FVector> locations, const bool returnFirstPositive)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::GetLightingAmount"));

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
		if ((!Itr->IsVisible()) || Itr->bHiddenInGame || Itr->GetOwner()->bHidden)
			continue;

		pointLights.Add(*Itr);
	}

	// We find local results for all locations
	for (FVector location : locations)
	{
		if (bShowDebug)
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
				/*if (bShowDebug)
					DrawDebugLine(gameWorld, location, lightLocation, FColor::Cyan);*/

				// 1 if near the edge of light, 0 if in center
				float temp = distance / lightRadius;
				// We take into account the inverse squared falloff
				// temp = FMath::Pow(temp, 0.5f);
				temp = FMath::Pow(temp, 2.f);
				// Now it's 0 near the edge and 1 in center
				temp = 1 - temp;

				// Finally we take intensity into account
				temp *= lightComp->Intensity / 150.f;

				// We also take color into account
				FLinearColor lightColor = lightComp->GetLightColor();
				temp *= FMath::Pow((lightColor.R * lightColor.R + lightColor.G * lightColor.G + lightColor.B * lightColor.B) / 3.0f, 0.35f); // 0.5f);

				// UE_LOG(LogTemp, Warning, TEXT("%f"), temp);
				// It always counts the brightest light
				if (temp > result)
				{
					result = temp;
					lightLoc = lightLocation;
					if (returnFirstPositive)
						return result;
				}
			}
		}
	}

	// UE_LOG(LogTemp, Warning, TEXT("Final %f"), result);
	return result;
}
// Returns true if one actor/location can see other actor/location
// Its not about visibility to human eye, doesn't take light into account
bool AMainGameMode::CanSee(const AActor * actor1, const AActor * actor2)
{
	if (!actor1 || !actor2)
		return false;

	return CanSee(actor1, actor1->GetActorLocation(), actor2, actor2->GetActorLocation());
}
bool AMainGameMode::CanSee(const FVector location1, const AActor * actor2)
{
	if (!actor2)
		return false;

	return CanSee(nullptr, location1, actor2, actor2->GetActorLocation());
}
bool AMainGameMode::CanSee(const AActor * actor1, const FVector location2)
{
	if (!actor1)
		return false;

	return CanSee(actor1, actor1->GetActorLocation(), nullptr, location2);
}
bool AMainGameMode::CanSee(const FVector location1, const FVector location2)
{
	return CanSee(nullptr, location1, nullptr, location2);
}
bool AMainGameMode::CanSee(const AActor * actor1, const FVector location1, const AActor * actor2)
{
	if (!actor2)
		return false;

	return CanSee(actor1, location1, actor2, actor2->GetActorLocation());
}
bool AMainGameMode::CanSee(const AActor * actor1, const AActor * actor2, const FVector location2)
{
	if (!actor1)
		return false;

	return CanSee(actor1, actor1->GetActorLocation(), actor2, location2);
}
bool AMainGameMode::CanSee(const AActor * actor1, const FVector location1, const FVector location2)
{
	return CanSee(actor1, location1, nullptr, location2);
}
bool AMainGameMode::CanSee(const FVector location1, const AActor * actor2, const FVector location2)
{
	return CanSee(nullptr, location1, actor2, location2);
}
bool AMainGameMode::CanSee(const AActor * actor1, const FVector location1, const AActor * actor2, const FVector location2)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::CanSee"));

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

	if (bShowDebug)
	{
		DrawDebugPoint(gameWorld, location1, 5, FColor::Red);
		DrawDebugPoint(gameWorld, location2, 5, FColor::Red);
	}

	bool bHit = gameWorld->LineTraceTestByChannel(location1, location2, ECC_Visibility, params);
	// TODO delete if affects performance too much
	// This is a line trace in a different direction helpful in cases when location1 is positioned inside something like a wall which is ignored by the line trace
	if (!bHit)
		bHit = gameWorld->LineTraceTestByChannel(location2, location1, ECC_Visibility, params);

	if (bShowDebug && !bHit)
		DrawDebugLine(gameWorld, location1, location2, FColor::Cyan);

	return !bHit;
}

// Returns the light level for a passage
float AMainGameMode::GetPassageLightingAmount(LabPassage * passage, bool oneSide, bool innerSide, const bool returnFirstPositive)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::GetPassageLightingAmount"));

	if (!passage)
		return 0.f;

	TArray<FVector> locations;

	float centerX, centerY;
	GridToWorld(passage->BotLeftX, passage->BotLeftY, passage->GridDirection == EDirectionEnum::VE_Up || passage->GridDirection == EDirectionEnum::VE_Down ? passage->Width : 1, passage->GridDirection == EDirectionEnum::VE_Up || passage->GridDirection == EDirectionEnum::VE_Down ? 1 : passage->Width, centerX, centerY);

	FVector center = FVector(centerX, centerY, 30); // Small offset to avoid floor

	// We create vectors for points assuming GridDirection is Up and innerSide is true
	// Then we rotate them based on actual direction and bool value
	TArray<FVector> localPoints;
	// localPoints.Add(FVector(-25, 0, 0));
	int step = 2; // TODO make constant
	for (int x = step; 2 * x - step < passage->Width; x += step)
	{
		localPoints.Add(FVector(-25, x * 50.f - step * 50.f / 2, 0));
		localPoints.Add(FVector(-25, -x * 50.f + step * 50.f / 2, 0));
	}

	switch (passage->GridDirection)
	{
	case EDirectionEnum::VE_Up:
		if (innerSide)
			break;
		for (int i = 0; i < localPoints.Num(); ++i)
			localPoints[i] = localPoints[i].RotateAngleAxis(180, FVector(0, 0, 1));
		break;
	case EDirectionEnum::VE_Down:
		if (!innerSide)
			break;
		for (int i = 0; i < localPoints.Num(); ++i)
			localPoints[i] = localPoints[i].RotateAngleAxis(180, FVector(0, 0, 1));
		break;
	case EDirectionEnum::VE_Right:
		for (int i = 0; i < localPoints.Num(); ++i)
			localPoints[i] = localPoints[i].RotateAngleAxis(innerSide ? 90 : -90, FVector(0, 0, 1));
		break;
	case EDirectionEnum::VE_Left:
		for (int i = 0; i < localPoints.Num(); ++i)
			localPoints[i] = localPoints[i].RotateAngleAxis(innerSide ? -90 : 90, FVector(0, 0, 1));
		break;
	}
	
	locations.Add(center);
	for (FVector localPoint : localPoints)
		locations.Add(center + localPoint);
	if(!oneSide)
		for (FVector localPoint : localPoints)
			locations.Add(center - localPoint);

	FVector lightLoc;	
	return GetLightingAmount(lightLoc, locations, returnFirstPositive);
}
// Returns true if passage is illuminated
bool AMainGameMode::IsPassageIlluminated(LabPassage * passage, bool oneSide, bool innerSide)
{
	return GetPassageLightingAmount(passage, oneSide, innerSide, true) > 0.f;
}
// Returns the light level for a room
float AMainGameMode::GetRoomLightingAmount(LabRoom * room, const bool returnFirstPositive)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::GetRoomLightingAmount"));

	if (!room)
		return 0.f;

	float light = 0.f;

	// for (LabPassage* passage : room->Passages)
	for (int i = 0; i < room->Passages.Num(); ++i)
	{
		LabPassage* passage = room->Passages[i];

		if (!passage)
			continue;

		// TODO shouldn't always be oneSide
		float temp = GetPassageLightingAmount(passage, true, passage->From == room, returnFirstPositive);
		light = temp > light ? temp : light;
		if (returnFirstPositive && light > 0.f)
			return light;

	}

	TArray<FVector> locations;
	int step = 2; // TODO make constant
	for (int x = step; x < room->SizeX - 1; x += step)
	{
		for (int y = step; y < room->SizeY - 1; y += step)
		{
			float pointX, pointY;
			GridToWorld(room->BotLeftX + x, room->BotLeftY + y, pointX, pointY);
			FVector point = FVector(pointX, pointY, 30); // Z is set to avoid interferance with floor

			locations.Add(point);
		}
	}
	FVector lightLoc;
	float temp = GetLightingAmount(lightLoc, locations, returnFirstPositive);
	light = temp > light ? temp : light;

	return light;
}
// Returns true if the room is in light
bool AMainGameMode::IsRoomIlluminated(LabRoom * room)
{
	return SpawnedRoomObjects.Contains(room) && GetRoomLightingAmount(room, true) > 0.f;
}

// Changes world location into grid location
void AMainGameMode::WorldToGrid(const float worldX, const float worldY, int & gridX, int & gridY)
{
	// We reverse x and y
	gridX = worldY / 50 + (worldY >= 0 ? 0 : -1);
	gridY = worldX / 50 + (worldX >= 0 ? 0 : -1);
}
// Changes grid location into world location
void AMainGameMode::GridToWorld(const int gridX, const int gridY, float & worldX, float & worldY)
{
	GridToWorld(gridX, gridY, 1, 1, worldX, worldY);
}
void AMainGameMode::GridToWorld(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, float & worldX, float & worldY)
{
	// We reverse x and y
	worldX = botLeftY * 50.f + sizeY * 25.f; // -25.f
	worldY = botLeftX * 50.f + sizeX * 25.f; // -25.f
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

// Returns by reference character's location on the grid
void AMainGameMode::GetCharacterLocation(int & x, int & y)
{
	FVector characterLocation = MainPlayerController->GetCharacter()->GetActorLocation();
	WorldToGrid(characterLocation.X, characterLocation.Y, x, y);
}
// Returns the room the character is in
LabRoom * AMainGameMode::GetCharacterRoom()
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::GetCharacterRoom"));

	int x, y;
	GetCharacterLocation(x, y);

	if (!ActualPlayerRoom || !PlayerRoom)
		MapSpaceIsFree(false, true, x, y, 1, 1, ActualPlayerRoom);
	else
	{
		// On left passage
		if (PlayerRoom->BotLeftX == x)
			MapSpaceIsFree(false, true, x - 1, y, 1, 1, ActualPlayerRoom);
		// On bottom passage
		else if (PlayerRoom->BotLeftY == y)
			MapSpaceIsFree(false, true, x, y - 1, 1, 1, ActualPlayerRoom);
		// On right passage
		else if (PlayerRoom->BotLeftX + PlayerRoom->SizeX - 1 == x)
			MapSpaceIsFree(false, true, x + 1, y, 1, 1, ActualPlayerRoom);
		// On top passage
		else if (PlayerRoom->BotLeftY + PlayerRoom->SizeY - 1 == y)
			MapSpaceIsFree(false, true, x, y + 1, 1, 1, ActualPlayerRoom);
		// Not on the passage
		else
			MapSpaceIsFree(false, true, x, y, 1, 1, ActualPlayerRoom);
	}
	/*if (!(ActualPlayerRoom && ActualPlayerRoom->BotLeftX <= x && ActualPlayerRoom->BotLeftY <= y && ActualPlayerRoom->BotLeftX + ActualPlayerRoom->SizeX - 1 >= x && ActualPlayerRoom->BotLeftY + ActualPlayerRoom->SizeY - 1 >= y))
		MapSpaceIsFree(false, true, x, y, 1, 1, ActualPlayerRoom);*/

	int d = 1; // Used to avoid situations when character is stuck in passage
	// Still in last room
	if (PlayerRoom && PlayerRoom->BotLeftX - d <= x && PlayerRoom->BotLeftY - d <= y && PlayerRoom->BotLeftX + PlayerRoom->SizeX - 1 + d >= x && PlayerRoom->BotLeftY + PlayerRoom->SizeY - 1 + d >= y)
		return PlayerRoom;

	if (ActualPlayerRoom)
		OnEnterRoom(); // PlayerRoom, characterRoom);

	return PlayerRoom;
}
// Called when character enters new room
void AMainGameMode::OnEnterRoom() // LabRoom* lastRoom, LabRoom* newRoom)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::OnEnterRoom"));

	LabRoom* lastRoom = PlayerRoom;
	PlayerRoom = ActualPlayerRoom;

	// UE_LOG(LogTemp, Warning, TEXT("Entered new room"));

	bool toActivateLamps = false;
	if (!VisitedRooms.Contains(PlayerRoom))
	{
		if (!lastRoom || RandBool(LampsTurnOnOnEnterProbability))
			toActivateLamps = true;
			// ActivateRoomLamps(PlayerRoom);
		VisitedRooms.Add(PlayerRoom);
		VisitedOverall++;
	}

	if (lastRoom)
	{
		// ReshapeDarkness(lastRoom, ReshapeDarknessDepth, false);

		// TODO smth about last room here
	}

	// CompleteReshapeAllDarknessAround();

	// We do it before reshaping cause we may accidentally trap ourselves if we turn light on after
	if (toActivateLamps)
		ActivateRoomLamps(PlayerRoom);

	if(RandBool(ReshapeDarknessOnEnterProbability))
		ReshapeAllDarkness();
	ExpandInDepth(PlayerRoom, ExpandDepth);
	SpawnFillInDepth(PlayerRoom, SpawnFillDepth);
}

// Called when character loses all of his lives
void AMainGameMode::OnLoss()
{
	if (!bHasWon)
	{
		UE_LOG(LogTemp, Warning, TEXT("You actually lost!"));

		GameHUD->ShowLossMessage();
	}
}
// Called when character is enabled to reset the map
void AMainGameMode::OnCharacterEnabled()
{
	ResetMap();
}
// Called when character reached the exit
void AMainGameMode::OnExitReached()
{
	bHasWon = true;

	UE_LOG(LogTemp, Warning, TEXT("Reached the exit!"));

	GameHUD->ShowVictoryMessage();

	// TODO stop character from moving?
}

// Called when character picks up an object to delete it from arrays without pooling
void AMainGameMode::OnPickUp(TScriptInterface<class IPickupable> object)
{
	AActor* actor = Cast<AActor>(object->_getUObject());
	if (!actor)
		return;

	FVector location = actor->GetActorLocation();
	int x, y;
	WorldToGrid(location.X, location.Y, x, y);
	LabRoom* room;
	if (!MapSpaceIsFree(false, true, x, y, 1, 1, room))
	{
		if (SpawnedRoomObjects.Contains(room))
			SpawnedRoomObjects[room].Remove(object->_getUObject());

		// TODO also deallocate room space
	}
}
// Called when exit door is opened to
void AMainGameMode::OnExitOpened(ABasicDoor* door)
{
	int x, y;
	FVector doorLoc = door->GetActorLocation();
	WorldToGrid(doorLoc.X, doorLoc.Y, x, y);

	if (door->GridDirection == EDirectionEnum::VE_Right)
		x++;
	else if (door->GridDirection == EDirectionEnum::VE_Left)
		x--;
	else if (door->GridDirection == EDirectionEnum::VE_Up)
		y++;
	else if (door->GridDirection == EDirectionEnum::VE_Down)
		y--;

	LabRoom* intersected;
	if (MapSpaceIsFree(false, true, x, y, 1, 1, intersected))
		return;

	// Trying to find exit volume
	for (TScriptInterface<IDeactivatable> object : SpawnedRoomObjects[intersected])
	{
		AExitVolume* exitVolume = Cast<AExitVolume>(object->_getUObject());
		if (!exitVolume)
			continue;

		exitVolume->ActivateLight();
		GameHUD->ShowHideWarning(true, FText::FromString("The light engulfs you, coming from the open exit. It's almost blinding. You feel a welcoming breeze from the passage ahead. Time to leave the lab behind"));
		break;
	}
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
	if (cl == DoorcardBP)
		return DoorcardPool;
	if (cl == ExitVolumeBP)
		return ExitVolumePool;
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
	DespawnRoom(room);

	AllocatedRoomSpace.Remove(room);
	AllocatedRooms.Remove(room);
	if (PlayerRoom == room)
		PlayerRoom = nullptr;
	if (ActualPlayerRoom == room)
		ActualPlayerRoom = nullptr;

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
	// Pool and clear all saved rooms
	TArray<LabRoom*> allRooms;
	AllocatedRoomSpace.GetKeys(allRooms);
	for (int i = allRooms.Num() - 1; i >= 0; --i)
		PoolRoom(allRooms[i]);

	// Should already be empty but we do this just in case
	SpawnedRoomObjects.Empty();
	AllocatedRoomSpace.Empty();
	ExpandedRooms.Empty();
	VisitedRooms.Empty();
	RoomsWithLampsOn.Empty();
	AllocatedRooms.Empty();
	PlayerRoom = nullptr;
	ActualPlayerRoom = nullptr;
	VisitedOverall = 0;
}

// Pools dark area returning all rooms that now need fixing
void AMainGameMode::PoolDarkness(LabRoom * start, int depth, TArray<LabRoom*>& toFix, bool stopAtFirstIfLit)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::PoolDarkness1"));

	if (!start)
		return;

	TArray<LabRoom*> toPool;
	PoolDarkness(start, depth, toFix, toPool, stopAtFirstIfLit);

	for (int i = toPool.Num() - 1; i >= 0; --i)
	{
		toFix.Remove(toPool[i]);
		PoolRoom(toPool[i]);
	}
}
void AMainGameMode::PoolDarkness(LabRoom * start, int depth, TArray<LabRoom*>& toFix, TArray<LabRoom*>& toPool, bool stopAtFirstIfLit)
{	
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::PoolDarkness2"));

	if (!start)
		return;

	// If we found a room that is not in the darkness, we add it for the fix and return, same if depth <= 0 or if character is in that room
	if (depth <= 0 || PlayerRoom == start || ActualPlayerRoom == start || IsRoomIlluminated(start))
	{
		toFix.AddUnique(start);
		if(depth <= 0 || stopAtFirstIfLit)
			return;
	}
	else
	{
		bool poolIt = true;
		// We check if room has exit cause if it does, we only want to delete it when both this room and the room behind exit are going to be pooled 
		for (LabPassage* passage : start->Passages)
		{
			// found exit
			if (passage->bIsDoor && passage->Width == ExitDoorWidth)
			{
				LabRoom* otherRoom = passage->To == start ? passage->From : passage->To;
				if (depth - 1 <= 0 || PlayerRoom == otherRoom || ActualPlayerRoom == otherRoom || IsRoomIlluminated(otherRoom))
				{
					poolIt = false;
					break;
				}
			}

		}
		if (poolIt)
			toPool.AddUnique(start);
		else
		{
			toFix.AddUnique(start);
			if (stopAtFirstIfLit)
				return;
		}
	}

	for (LabPassage* passage : start->Passages)
	{
		if (passage->From != start)
			PoolDarkness(passage->From, depth - 1, toFix, toPool, stopAtFirstIfLit); // !!
		else if (passage->To != start)
			PoolDarkness(passage->To, depth - 1, toFix, toPool, stopAtFirstIfLit); // !!
	}
}

// Pools dark area and fixes every room that needs fixing
void AMainGameMode::ReshapeDarkness(LabRoom * start, int depth, bool stopAtFirstIfLit)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::ReshapeDarkness"));

	TArray<LabRoom*> toFix;
	PoolDarkness(start, depth, toFix, stopAtFirstIfLit);
	for (LabRoom* roomToFix : toFix)
		FixRoom(roomToFix);
}
// Reshapes darkness and expands, spawns and fills rooms
void AMainGameMode::CompleteReshapeDarkness(LabRoom * start, bool stopAtFirstIfLit)
{
	ReshapeDarkness(start, ReshapeDarknessDepth, stopAtFirstIfLit);
	ExpandInDepth(start, ExpandDepth);
	SpawnFillInDepth(start, SpawnFillDepth);
}
// Reshapes darkness in player room
void AMainGameMode::CompleteReshapeDarknessAround()
{
	if (!PlayerRoom)
		return;

	CompleteReshapeDarkness(PlayerRoom, false);
}
// Pools all dark rooms on the map and fixes every room that needs fixing
void AMainGameMode::ReshapeAllDarkness()
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::ReshapeAllDarkness"));

	TArray<LabRoom*> allRooms;
	AllocatedRoomSpace.GetKeys(allRooms);	

	TArray<LabRoom*> toPool;
	TArray<LabRoom*> toFix;

	for (LabRoom* room : allRooms)
	{
		// If we found a room that is not in the darkness, we add it for the fix and continue, same if character is in that room
		if (PlayerRoom == room || ActualPlayerRoom == room || IsRoomIlluminated(room))
			toFix.AddUnique(room);
		else
		{
			bool poolIt = true;
			// We check if room has exit cause if it does, we only want to delete it when both this room and the room behind exit are going to be pooled 
			for (LabPassage* passage : room->Passages)
			{
				// found exit
				if (passage->bIsDoor && passage->Width == ExitDoorWidth)
				{
					LabRoom* otherRoom = passage->To == room ? passage->From : passage->To;
					if (PlayerRoom == otherRoom || ActualPlayerRoom == otherRoom || IsRoomIlluminated(otherRoom))
					{
						poolIt = false;
						break;
					}
				}
			}
			if (poolIt)
				toPool.AddUnique(room);
			else
				toFix.AddUnique(room);
		}
	}

	for (int i = toPool.Num() - 1; i >= 0; --i)
		PoolRoom(toPool[i]);
	
	// We want player's room to be fixed first so nothing interferes with it
	FixRoom(PlayerRoom);
	FixRoom(ActualPlayerRoom);
	for (LabRoom* roomToFix : toFix)
		FixRoom(roomToFix);
}
// Reshapes all darkness and also expands spawns and fills around player's room
void AMainGameMode::CompleteReshapeAllDarknessAround()
{
	if (!PlayerRoom)
		return;

	ReshapeAllDarkness();
	ExpandInDepth(PlayerRoom, ExpandDepth);
	SpawnFillInDepth(PlayerRoom, SpawnFillDepth);
}
// Calls CompleteReshapeAllDarknessAround with specified probability
void AMainGameMode::CompleteReshapeAllDarknessAroundOnTick()
{
	if (RandBool(ReshapeDarknessOnTickProbability))
		CompleteReshapeAllDarknessAround();
}

// Tries to find a poolable object
UObject* AMainGameMode::TryGetPoolable(UClass* cl)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::TryGetPoolable"));

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

	if (index < 0)
		return nullptr;

	object = pool[index];
	UObject* obj = object->_getUObject();
	// object->Execute_SetActive(obj, true);
	pool.RemoveAt(index);
	return obj;
}

// Spawn specific objects
ABasicFloor* AMainGameMode::SpawnBasicFloor(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabRoom* room)
{
	ABasicFloor* floor = Cast<ABasicFloor>(TryGetPoolable(BasicFloorBP));
	if (!floor)
	{
		floor = GetWorld()->SpawnActor<ABasicFloor>(BasicFloorBP);
		floor->Execute_SetActive(floor, false);
	}

	PlaceObject(floor, botLeftX, botLeftY, sizeX, sizeY);
	floor->Execute_SetActive(floor, true);

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
	{
		wall = GetWorld()->SpawnActor<ABasicWall>(BasicWallBP);
		wall->Execute_SetActive(wall, false);
	}

	PlaceObject(wall, botLeftX, botLeftY, sizeX, sizeY);
	wall->Execute_SetActive(wall, true);

	if (room && SpawnedRoomObjects.Contains(room))
		SpawnedRoomObjects[room].Add(wall);

	// UE_LOG(LogTemp, Warning, TEXT("Spawned a basic wall"));

	return wall;
}
ABasicDoor * AMainGameMode::SpawnBasicDoor(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color, const int width, LabPassage* passage)
{
	ABasicDoor* door = Cast<ABasicDoor>(TryGetPoolable(BasicDoorBP));
	if (!door)
	{
		door = GetWorld()->SpawnActor<ABasicDoor>(BasicDoorBP);
		door->Execute_SetActive(door, false);
	}

	door->ResetDoor(width == ExitDoorWidth); // Clothes the door if it was open
	door->DoorColor = color; // Sets door's color
	PlaceObject(door, botLeftX, botLeftY, direction, width);
	door->Execute_SetActive(door, true);

	if (passage && SpawnedPassageObjects.Contains(passage))
		SpawnedPassageObjects[passage].Add(door);

	// UE_LOG(LogTemp, Warning, TEXT("Spawned a basic door"));

	return door;
}
AWallLamp * AMainGameMode::SpawnWallLamp(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color, const int width, LabRoom* room)
{
	AWallLamp* lamp = Cast<AWallLamp>(TryGetPoolable(WallLampBP));
	if (!lamp)
	{
		lamp = GetWorld()->SpawnActor<AWallLamp>(WallLampBP);
		lamp->Execute_SetActive(lamp, false);
	}

	lamp->Reset(); // Disables light if it was on
	lamp->SetColor(color); // Sets correct color
	PlaceObject(lamp, botLeftX, botLeftY, direction, width);
	lamp->Execute_SetActive(lamp, true);

	if (room)
	{
		if (SpawnedRoomObjects.Contains(room))
			SpawnedRoomObjects[room].Add(lamp);
		if (AllocatedRoomSpace.Contains(room))
			AllocateRoomSpace(room, botLeftX, botLeftY, direction, width, false);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Spawned a wall lamp"));

	return lamp;
}
AFlashlight* AMainGameMode::SpawnFlashlight(const int botLeftX, const int botLeftY, const EDirectionEnum direction, LabRoom* room)
{
	AFlashlight* flashlight = Cast<AFlashlight>((TryGetPoolable(FlashlightBP)));
	if (!flashlight)
	{
		flashlight = GetWorld()->SpawnActor<AFlashlight>(FlashlightBP);
		flashlight->Execute_SetActive(flashlight, false);
	}

	flashlight->Reset(); // Disables light if it was on
	PlaceObject(flashlight, botLeftX, botLeftY, direction);
	flashlight->Execute_SetActive(flashlight, true);

	if (room)
	{
		if (SpawnedRoomObjects.Contains(room))
			SpawnedRoomObjects[room].Add(flashlight);
		if (AllocatedRoomSpace.Contains(room))
			AllocateRoomSpace(room, botLeftX, botLeftY, 1, 1, false);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Spawned a flashlight"));

	return flashlight;
}
ALighter * AMainGameMode::SpawnLighter(const int botLeftX, const int botLeftY, const EDirectionEnum direction, LabRoom * room)
{
	//ALighter* lighter = Cast<ALighter>((TryGetPoolable(LighterBP)));
	//if (!lighter)
	//{
	ALighter* lighter = GetWorld()->SpawnActor<ALighter>(LighterBP);
	lighter->Execute_SetActive(lighter, false);
	//}

	lighter->Reset(); // Disables light if it was on
	PlaceObject(lighter, botLeftX, botLeftY, direction);
	lighter->Execute_SetActive(lighter, true);

	if (room)
	{
		if (SpawnedRoomObjects.Contains(room))
			SpawnedRoomObjects[room].Add(lighter);
		if (AllocatedRoomSpace.Contains(room))
			AllocateRoomSpace(room, botLeftX, botLeftY, 1, 1, false);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Spawned a lighter"));

	return lighter;
}
ADoorcard* AMainGameMode::SpawnDoorcard(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color, LabRoom* room)
{
	ADoorcard* doorcard = Cast<ADoorcard>((TryGetPoolable(DoorcardBP)));
	if (!doorcard)
	{
		doorcard = GetWorld()->SpawnActor<ADoorcard>(DoorcardBP);
		doorcard->Execute_SetActive(doorcard, false);
	}

	doorcard->SetColor(color); // Sets correct color
	PlaceObject(doorcard, botLeftX, botLeftY, direction);
	doorcard->Execute_SetActive(doorcard, true);

	if (room)
	{
		if (SpawnedRoomObjects.Contains(room))
			SpawnedRoomObjects[room].Add(doorcard);
		if (AllocatedRoomSpace.Contains(room))
			AllocateRoomSpace(room, botLeftX, botLeftY, 1, 1, false);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Spawned a flashlight"));

	return doorcard;
}
AExitVolume * AMainGameMode::SpawnExitVolume(const int botLeftX, const int botLeftY, const EDirectionEnum direction, LabRoom * room)
{
	AExitVolume* exit = Cast<AExitVolume>(TryGetPoolable(ExitVolumeBP));
	if (!exit)
	{
		exit = GetWorld()->SpawnActor<AExitVolume>(ExitVolumeBP);
		exit->Execute_SetActive(exit, false);
	}

	exit->Reset(); // Disables light if it was on
	PlaceObject(exit, botLeftX, botLeftY, direction, ExitDoorWidth);
	exit->Execute_SetActive(exit, true);

	if (room && SpawnedRoomObjects.Contains(room))
		SpawnedRoomObjects[room].Add(exit);

	// UE_LOG(LogTemp, Warning, TEXT("Spawned an exit volume"));

	return nullptr;
}

// Spawn full parts of the lab
void AMainGameMode::SpawnRoom(LabRoom * room)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::SpawnRoom"));

	if (!room)
		return;

	// We don't spawn one room twice
	if (SpawnedRoomObjects.Contains(room))
		return;

	/*if (!PlayerRoom || room == PlayerRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), (!room->Passages[0]->bIsDoor ? TEXT("YES") : TEXT("NO")));
	}*/

	DeallocateRoom(room);
	SpawnedRoomObjects.Add(room);

	// Spawning floor
	// Doesn't include walls and passages
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

	// UE_LOG(LogTemp, Warning, TEXT("Spawned a room"));
}
void AMainGameMode::SpawnPassage(LabPassage* passage, LabRoom* room)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::SpawnPassage"));

	if (!passage)
		return;

	// We don't spawn one passage twice
	if (SpawnedPassageObjects.Contains(passage))
		return;

	SpawnedPassageObjects.Add(passage);

	// Spawns floor under the passage
	if (passage->GridDirection == EDirectionEnum::VE_Up || passage->GridDirection == EDirectionEnum::VE_Down)
		SpawnBasicFloor(passage->BotLeftX, passage->BotLeftY, passage->Width, 1, passage);
	else
		SpawnBasicFloor(passage->BotLeftX, passage->BotLeftY, 1, passage->Width, passage);

	// Spawn door if needed
	if (passage->bIsDoor)
	{
		SpawnBasicDoor(passage->BotLeftX, passage->BotLeftY, passage->GridDirection, passage->Color, passage->Width, passage);
	}

	// UE_LOG(LogTemp, Warning, TEXT("> Spawned a passage"));
}
// Despawns room so it can be respawned later
void AMainGameMode::DespawnRoom(LabRoom * room)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::DespawnRoom"));

	if (!room)
		return;

	if (SpawnedRoomObjects.Contains(room))
	{
		PoolObjects(SpawnedRoomObjects[room]);
		for (LabPassage* passage : room->Passages)
		{
			if (!passage)
				continue;

			// We pool passage if it isn't connected to some other spawned room
			if (passage->From == room)
			{
				if (!passage->To || !SpawnedRoomObjects.Contains(passage->To))
					PoolPassage(passage);
			}
			else if (passage->To == room)
			{
				if (!passage->From || !SpawnedRoomObjects.Contains(passage->From))
					PoolPassage(passage);
			}
			// else
			// This is a weird case that should never happen
			// Room is not responsible for the passage pooling is this DOES happen somehow
		}
	}
	SpawnedRoomObjects.Remove(room);
	AllocatedRoomSpace[room].Empty();
	ExpandedRooms.Remove(room);
	VisitedRooms.Remove(room); // ?
	RoomsWithLampsOn.Remove(room);
	AllocateRoom(room);
}

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

	if (!room || !AllocatedRoomSpace.Contains(room))
		return;
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
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::MapSpaceIsFree"));

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
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::RoomSpaceIsFree"));

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

// Returns true is one intersects the other (more than just side)
bool AMainGameMode::Intersect(LabRoom * room1, LabRoom * room2)
{
	if (!room1 || !room2)
		return false;

	return Intersect(FRectSpaceStruct(room1->BotLeftX, room1->BotLeftY, room1->SizeX, room1->SizeY), FRectSpaceStruct(room2->BotLeftX, room2->BotLeftY, room2->SizeX, room2->SizeY));
}
bool AMainGameMode::Intersect(FRectSpaceStruct space1, LabRoom * room2)
{
	if (!room2)
		return false;

	return Intersect(space1, FRectSpaceStruct(room2->BotLeftX, room2->BotLeftY, room2->SizeX, room2->SizeY));
}
bool AMainGameMode::Intersect(LabRoom * room1, FRectSpaceStruct space2)
{
	if (!room1)
		return false;

	return Intersect(FRectSpaceStruct(room1->BotLeftX, room1->BotLeftY, room1->SizeX, room1->SizeY), space2);
}
bool AMainGameMode::Intersect(FRectSpaceStruct space1, FRectSpaceStruct space2)
{
	// Not intersecting on X axis
	if (space1.BotLeftX + space1.SizeX - 1 <= space2.BotLeftX)
		return false;
	if (space1.BotLeftX >= space2.BotLeftX + space2.SizeX - 1)
		return false;

	// Not intersecting on Y axis
	if (space1.BotLeftY + space1.SizeY - 1 <= space2.BotLeftY)
		return false;
	if (space1.BotLeftY >= space2.BotLeftY + space2.SizeY - 1)
		return false;

	// Intersecting on both axis
	return true;
}

// Returns true is first is inside second
bool AMainGameMode::IsInside(LabRoom * room1, LabRoom * room2)
{
	if (!room1 || !room2)
		return false;

	return IsInside(FRectSpaceStruct(room1->BotLeftX, room1->BotLeftY, room1->SizeX, room1->SizeY), FRectSpaceStruct(room2->BotLeftX, room2->BotLeftY, room2->SizeX, room2->SizeY));
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
	if (space1.BotLeftX < space2.BotLeftX)
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

	// Everything is fine
	return true;
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
	FRectSpaceStruct minSpace(-3, -3, 7, 7);

	return CreateRandomRoom(minSpace);;
}

// Reverses direction
EDirectionEnum AMainGameMode::GetReverseDirection(EDirectionEnum direction)
{
	switch (direction)
	{
	case EDirectionEnum::VE_Down:
		return EDirectionEnum::VE_Up;
	case EDirectionEnum::VE_Left:
		return EDirectionEnum::VE_Right;
	case EDirectionEnum::VE_Right:
		return EDirectionEnum::VE_Left;
	case EDirectionEnum::VE_Up:
		return EDirectionEnum::VE_Down;
	}
	UE_LOG(LogTemp, Warning, TEXT("Somehow reached the end of GetReverseDirection"))
	return EDirectionEnum::VE_Up;
}

// Creates random space for a future passage (not world location but offsets)
// Doesn't take other passages into account. Direction is always out
FRectSpaceStruct AMainGameMode::CreateRandomPassageSpace(LabRoom * room, EDirectionEnum& direction, const bool forDoor)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::CreateRandomPassageSpace"));

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

	int doorWidth = !forDoor ? 
		0 : 
		(VisitedOverall < MinVisitedBeforeExitCanSpawn || !RandBool(DoorIsExitProbability) ?
			(RandBool(DoorIsNormalProbability) ? 
				NormalDoorWidth : 
				BigDoorWidth) :
			ExitDoorWidth);
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
// Creates a passage space for existing passage
FRectSpaceStruct AMainGameMode::CreatePassageSpaceFromPassage(LabRoom* room, LabPassage * passage)
{
	return FRectSpaceStruct(passage->BotLeftX - room->BotLeftX, passage->BotLeftY - room->BotLeftY, passage->GridDirection == EDirectionEnum::VE_Up || passage->GridDirection == EDirectionEnum::VE_Down ? passage->Width : 1, passage->GridDirection == EDirectionEnum::VE_Up || passage->GridDirection == EDirectionEnum::VE_Down ? 1 : passage->Width);
}

// Creates minimum space for a room near passage space for tests and allocation
// TODO maybe it should take room size just in case other room gets destroyed
FRectSpaceStruct AMainGameMode::CreateMinimumRoomSpace(LabRoom* room, FRectSpaceStruct passageSpace, EDirectionEnum direction, bool widerForDoor)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::CreateMinimumRoomSpace1"));

	FRectSpaceStruct space;

	int width = direction == EDirectionEnum::VE_Left || direction == EDirectionEnum::VE_Right ? passageSpace.SizeY : passageSpace.SizeX;
	int delta = !widerForDoor ? MinDistanceBetweenPassages : FMath::Max(MinDistanceBetweenPassages, width / 2 + width % 2);
	// int delta = MinDistanceBetweenPassages;

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
// Creates minimum space for a room near passage for tests and allocation
FRectSpaceStruct AMainGameMode::CreateMinimumRoomSpace(LabRoom * room, LabPassage * passage)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::CreateMinimumRoomSpace2"));

	EDirectionEnum direction = !passage->To ? passage->GridDirection : GetReverseDirection(passage->GridDirection);

	FRectSpaceStruct pasSpace = CreatePassageSpaceFromPassage(room, passage);
	bool doorOutOfRoomBorders = false;
	if (passage->bIsDoor)
	{
		int extra = passage->Width / 2 + passage->Width % 2;
		if (direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down)
			doorOutOfRoomBorders = room->BotLeftX > passage->BotLeftX - extra || room->BotLeftX + room->SizeX < passage->BotLeftX + passage->Width + extra;
		else
			doorOutOfRoomBorders = room->BotLeftY > passage->BotLeftY - extra || room->BotLeftY + room->SizeY < passage->BotLeftY + passage->Width + extra;
	}
	return CreateMinimumRoomSpace(room, pasSpace, direction, doorOutOfRoomBorders);
}

// Creates random room space based on minimum room space
FRectSpaceStruct AMainGameMode::CreateRandomRoomSpace(FRectSpaceStruct minSpace, bool fromPassage, EDirectionEnum direction)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::CreateRandomRoomSpace"));
	
	FRectSpaceStruct randomSpace;

	int area = FMath::RandRange(FMath::Max(minSpace.SizeX * minSpace.SizeY, MinRoomArea), MaxRoomArea);

	// We randomize what we make first
	if (FMath::RandBool())
	{
		randomSpace.SizeX = FMath::RandRange(minSpace.SizeX, FMath::Min(area / minSpace.SizeY, MaxRoomSize));
		randomSpace.SizeY = FMath::Max(minSpace.SizeY, area / randomSpace.SizeX);
	}
	else
	{
		randomSpace.SizeY = FMath::RandRange(minSpace.SizeY, FMath::Min(area / minSpace.SizeX, MaxRoomSize));
		randomSpace.SizeX = FMath::Max(minSpace.SizeX, area / randomSpace.SizeY);
	}

	if (!fromPassage)
	{
		randomSpace.BotLeftX = FMath::RandRange(minSpace.BotLeftX - randomSpace.SizeX + minSpace.SizeX, minSpace.BotLeftX);
		randomSpace.BotLeftY = FMath::RandRange(minSpace.BotLeftY - randomSpace.SizeY + minSpace.SizeY, minSpace.BotLeftY);
	}
	else
	{
		if (direction == EDirectionEnum::VE_Left || direction == EDirectionEnum::VE_Right)
		{
			randomSpace.BotLeftY = FMath::RandRange(minSpace.BotLeftY - randomSpace.SizeY + minSpace.SizeY, minSpace.BotLeftY);
			if (direction == EDirectionEnum::VE_Right)
				randomSpace.BotLeftX = minSpace.BotLeftX;
			else
				randomSpace.BotLeftX = minSpace.BotLeftX - randomSpace.SizeX + minSpace.SizeX;
		}
		else
		{
			randomSpace.BotLeftX = FMath::RandRange(minSpace.BotLeftX - randomSpace.SizeX + minSpace.SizeX, minSpace.BotLeftX);
			if (direction == EDirectionEnum::VE_Up)
				randomSpace.BotLeftY = minSpace.BotLeftY;
			else
				randomSpace.BotLeftY = minSpace.BotLeftY - randomSpace.SizeY + minSpace.SizeY;
		}
	}

	return randomSpace;
}

// Shrinks space to not include specified room but still include minSpace
// If prioritize horizontal is true, it tries to shrink horizontal part, otherwise vertival
bool AMainGameMode::ShrinkSpace(FRectSpaceStruct & currentSpace, FRectSpaceStruct minSpace, LabRoom * toAvoid, bool prioritizeX)
{
	if (!toAvoid)
		return false;

	if (prioritizeX)
		return TryShrinkX(currentSpace, minSpace, toAvoid)
			|| TryShrinkY(currentSpace, minSpace, toAvoid);
	else
		return TryShrinkY(currentSpace, minSpace, toAvoid)
			|| TryShrinkX(currentSpace, minSpace, toAvoid);
}
bool AMainGameMode::TryShrinkX(FRectSpaceStruct & currentSpace, FRectSpaceStruct minSpace, LabRoom * toAvoid)
{
	// Check that intersect on X exists 
	if (currentSpace.BotLeftX + currentSpace.SizeX - 1 > toAvoid->BotLeftX && 
		toAvoid->BotLeftX + toAvoid->SizeX - 1 > currentSpace.BotLeftX)
	{
		// minSpace is to the left of toAvoid
		if (minSpace.BotLeftX + minSpace.SizeX - 1 <= toAvoid->BotLeftX)
		{
			currentSpace.SizeX = toAvoid->BotLeftX - currentSpace.BotLeftX + 1;
			return true;
		}
		// minSpace is to the right of toAvoid
		else if (toAvoid->BotLeftX + toAvoid->SizeX - 1 <= minSpace.BotLeftX)
		{
			int delta = toAvoid->BotLeftX + toAvoid->SizeX - 1 - currentSpace.BotLeftX;
			currentSpace.BotLeftX += delta;
			currentSpace.SizeX -= delta;
			return true;
		}
		// Intersects minSpace, no solution exists
		else
			return false;
	}
	// Does not intersect currentSpace and thus there is no problem to solve
	// This shouldn't happen if sent data is correct (intersecting)
	else
		return false;
}
bool AMainGameMode::TryShrinkY(FRectSpaceStruct & currentSpace, FRectSpaceStruct minSpace, LabRoom * toAvoid)
{
	// Check that intersect on Y exists 
	if (currentSpace.BotLeftY + currentSpace.SizeY - 1 > toAvoid->BotLeftY &&
		toAvoid->BotLeftY + toAvoid->SizeY - 1 > currentSpace.BotLeftY)
	{
		// minSpace is below the toAvoid
		if (minSpace.BotLeftY + minSpace.SizeY - 1 <= toAvoid->BotLeftY)
		{
			currentSpace.SizeY = toAvoid->BotLeftY - currentSpace.BotLeftY + 1;
			return true;
		}
		// minSpace is above the toAvoid
		else if (toAvoid->BotLeftY + toAvoid->SizeY - 1 <= minSpace.BotLeftY)
		{
			int delta = toAvoid->BotLeftY + toAvoid->SizeY - 1 - currentSpace.BotLeftY;
			currentSpace.BotLeftY += delta;
			currentSpace.SizeY -= delta;
			return true;
		}
		// Intersects minSpace, no solution exists
		else
			return false;
	}
	// Does not intersect currentSpace and thus there is no problem to solve
	// This shouldn't happen if sent data is correct (intersecting)
	else
		return false;
}

// Creates a random room based on minimum room space
LabRoom * AMainGameMode::CreateRandomRoom(FRectSpaceStruct minSpace, bool fromPassage, EDirectionEnum direction, bool keepMinimum)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::CreateRandomRoom"));

	if (keepMinimum)
		return CreateRoom(minSpace);
	
	FRectSpaceStruct currentSpace = CreateRandomRoomSpace(minSpace, fromPassage, direction);
	// TArray<LabPassage*> additionalPassages;

	LabRoom* intersected;
	bool prioritizeX = FMath::RandBool(); // TODO should depend on smth
	bool swapPriority = false;

	while (!MapSpaceIsFree(true, false, currentSpace, intersected) && intersected)
	{
		UE_LOG(LogTemp, Warning, TEXT("> Shrinking among allocated"));

		// TODO should try to include intersected and add its passages
		// IsInside()
		
		UE_LOG(LogTemp, Warning, TEXT("> x: %d, y: %d, sX: %d, sY: %d"), currentSpace.BotLeftX, currentSpace.BotLeftY, currentSpace.SizeX, currentSpace.SizeY);
		if (!ShrinkSpace(currentSpace, minSpace, intersected, prioritizeX))
		{
			UE_LOG(LogTemp, Warning, TEXT("> Failed to shrink"));
			return nullptr;
		}
		UE_LOG(LogTemp, Warning, TEXT("> x: %d, y: %d, sX: %d, sY: %d"), currentSpace.BotLeftX, currentSpace.BotLeftY, currentSpace.SizeX, currentSpace.SizeY);

		if (swapPriority)
			prioritizeX = !prioritizeX;
		else
			prioritizeX = FMath::RandBool();
	}
	while (!MapSpaceIsFree(false, true, currentSpace, intersected) && intersected)
	{
		UE_LOG(LogTemp, Warning, TEXT("> Shrinking among spawned"));

		UE_LOG(LogTemp, Warning, TEXT("> x: %d, y: %d, sX: %d, sY: %d"), currentSpace.BotLeftX, currentSpace.BotLeftY, currentSpace.SizeX, currentSpace.SizeY);
		if (!ShrinkSpace(currentSpace, minSpace, intersected, prioritizeX))
		{
			UE_LOG(LogTemp, Warning, TEXT("> Failed to shrink"));
			return nullptr;
		}
		UE_LOG(LogTemp, Warning, TEXT("> x: %d, y: %d, sX: %d, sY: %d"), currentSpace.BotLeftX, currentSpace.BotLeftY, currentSpace.SizeX, currentSpace.SizeY);

		if (swapPriority)
			prioritizeX = !prioritizeX;
		else
			prioritizeX = FMath::RandBool();
	}

	LabRoom* room = CreateRoom(currentSpace);

	return room;
}

// Creates and adds a random passage to the room, returns passage or nullptr, also allocates room space and returns allocated room space by reference
LabPassage * AMainGameMode::CreateAndAddRandomPassage(LabRoom * room, FRectSpaceStruct & roomSpace, LabRoom*& possibleRoomConnection)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::CreateAndAddRandomPassage"));

	// bool roomIsSpawned = SpawnedRoomObjects.Contains(room);

	bool forDoor = RandBool(PassageIsDoorProbability);
	EDirectionEnum direction;

	// Find random position for the new passage
	FRectSpaceStruct pasSpace = CreateRandomPassageSpace(room, direction, forDoor);
	// Test if it works in the room
	if (!RoomSpaceIsFree(room, pasSpace, true, forDoor))
		// || (roomIsSpawned 
		// 	&& (IsPassageIlluminated(&LabPassage(room->BotLeftX + pasSpace.BotLeftX, room->BotLeftY + pasSpace.BotLeftY, direction)) 
		//		|| )))
		return nullptr;

	bool pasIsExit = forDoor && (pasSpace.SizeX == ExitDoorWidth || pasSpace.SizeY == ExitDoorWidth);

	// Find minimum space for a new room on the other side of this passage
	roomSpace = CreateMinimumRoomSpace(room, pasSpace, direction);

	LabRoom* intersected = nullptr;
	// Intersects something spawned
	if (!MapSpaceIsFree(false, true, roomSpace, intersected))
	{
		// return nullptr;

		// Exit is only created as a new room
		if (pasIsExit)
			return nullptr;

		// If we don't want to connect
		if (!RandBool(ConnectToOtherRoomProbability))
			return nullptr;

		// We found something that intersects roomSpace and is spawned
		// We check if instead of creating new room (which we can't do since we intersected) we can connect to this room instead

		// Room shouldn't be illuminated
		if (PlayerRoom == intersected || ActualPlayerRoom == intersected || IsRoomIlluminated(intersected))
			return nullptr;

		// Room shouldn't be inner side of the exit
		for (LabPassage* interPas : intersected->Passages)
		{
			bool interPasIsExit = interPas->bIsDoor && interPas->To == intersected && interPas->Width == ExitDoorWidth;
			if (interPasIsExit)
				return nullptr;
		}
		
		// Other room should include the room space
		if (!IsInside(roomSpace, intersected))
			return nullptr;

		// Despawn that room so it can be respawned later
		DespawnRoom(intersected);

		// At this point other room should be considered good
		possibleRoomConnection = intersected;
	}
	else
	{
		intersected = nullptr;
		// Intersects something allocated
		bool spaceIsFree = MapSpaceIsFree(true, false, roomSpace, intersected);
		if (!spaceIsFree)
		{
			// Exit is only created as a new room
			if (pasIsExit)
				return nullptr;

			// If we don't want to connect
			if (!RandBool(ConnectToOtherRoomProbability))
				return nullptr;

			// We found something that intersects roomSpace but is not spawned yet
			// We check if instead of creating new room (which we can't do since we intersected) we can connect to this room instead

			// Room shouldn't be inner side of the exit
			for (LabPassage* interPas : intersected->Passages)
			{
				bool interPasIsExit = interPas->bIsDoor && interPas->To == intersected && interPas->Width == ExitDoorWidth;
				if (interPasIsExit)
					return nullptr;
			}

			// Other room should include the room space
			if (!IsInside(roomSpace, intersected))
				return nullptr;

			// TODO maybe we should delete this since other room should always be able to include passage if it includes roomSpace
			// Now we check if other room can include our passage
			FRectSpaceStruct pasSpaceForOther = FRectSpaceStruct(pasSpace.BotLeftX - intersected->BotLeftX + room->BotLeftX, pasSpace.BotLeftY - intersected->BotLeftY + room->BotLeftY, pasSpace.SizeX, pasSpace.SizeY);
			if (!RoomSpaceIsFree(intersected, pasSpaceForOther, true)) // We don't check for door since we already have good walls for sliding door in original room
				return nullptr;

			// At this point other room should be considered good
			possibleRoomConnection = intersected;
		}
	}

	// Add this passage to the room
	LabPassage* passage;
	if (!forDoor)
		passage = room->AddPassage(room->BotLeftX + pasSpace.BotLeftX, room->BotLeftY + pasSpace.BotLeftY, direction, possibleRoomConnection, direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down ? pasSpace.SizeX : pasSpace.SizeY);
	else
	{
		// TODO maybe it shouldn't allow every color
		FLinearColor color = !pasIsExit ? RandColor() : FLinearColor::Black;

		passage = room->AddPassage(room->BotLeftX + pasSpace.BotLeftX, room->BotLeftY + pasSpace.BotLeftY, direction, possibleRoomConnection, forDoor, color, direction == EDirectionEnum::VE_Up || direction == EDirectionEnum::VE_Down ? pasSpace.SizeX : pasSpace.SizeY);
	}

	// if (roomIsSpawned)
	// 	RespawnRoomWalls(room); // Doesn't spawn new passage

	return passage;
}

// Creates new passages in the room
// Create new rooms for passages 
// Returns new rooms
TArray<LabRoom*> AMainGameMode::ExpandRoom(LabRoom * room, int desiredNumOfPassagesOverride)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::ExpandRoom"));

	TArray<LabRoom*> newRooms;

	if (!room)
		return newRooms;

	ExpandedRooms.AddUnique(room);

	// Room shouldn't be inner side of the exit
	for (LabPassage* interPas : room->Passages)
	{
		if (!interPas)
			continue;
		bool interPasIsExit = interPas->bIsDoor && interPas->To == room && interPas->Width == ExitDoorWidth;
		if (interPasIsExit)
			return newRooms;
	}
	UE_LOG(LogTemp, Warning, TEXT("> Not inner exit"));

	//if (desiredNumOfPassagesOverride < MinRoomNumOfPassages && room->Passages.Num() == 1 && room->Passages[0]->bIsDoor && room->Passages[0]->Color != FLinearColor::White && RandBool(MakeRoomSpecialForCardProbability))
	//{
	//	// We don't expand so this room becomes a vault for card
	//	return newRooms;
	//}

	// The number of passages we want to have in the room
	// These are not just new but overall
	int desiredNumOfPassages = desiredNumOfPassagesOverride < MinRoomNumOfPassages ?FMath::RandRange(MinRoomNumOfPassages, MaxRoomNumOfPassages) : desiredNumOfPassagesOverride;
	// UE_LOG(LogTemp, Warning, TEXT("> Trying to add %d passages"), desiredNumOfPassages);

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
			// TODO it shouldn't be like this
			// For start room
			if (desiredNumOfPassagesOverride >= MinRoomNumOfPassages && !SpawnedPassageObjects.Contains(passage))
			{
				// UE_LOG(LogTemp, Warning, TEXT("Made it doorless"));
				// passage->bIsDoor = true; // false;
				// UE_LOG(LogTemp, Warning, TEXT("Made it white"));
				passage->Color = FLinearColor::White;
			}

			if (!possibleRoomConnection)
			{
				bool pasIsExit = passage->bIsDoor && passage->Width == ExitDoorWidth;

				// UE_LOG(LogTemp, Warning, TEXT("> %s"), TEXT("success"));

				// We create new room from min space, it also allocates room's space
				LabRoom* newRoom = CreateRandomRoom(minRoomSpace, true, passage->GridDirection, pasIsExit);
				if (!newRoom)
					continue;

				// We add passage to the room
				newRoom->AddPassage(passage);
				newRooms.Add(newRoom);
			}
			else
			{
				// UE_LOG(LogTemp, Warning, TEXT("> %s"), TEXT("found a room to connect to"));
			}
		}
		else
		{
			// UE_LOG(LogTemp, Warning, TEXT("> %s"), TEXT("failure"));
		}
	}

	return newRooms;
}

// Fixes room's passages that lead nowhere, creating a room for them or deleting them
// Also spawns a wall over previous passage if room was spawned
void AMainGameMode::FixRoom(LabRoom * room, int depth)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::FixRoom"));

	if (depth > MaxFixDepth)
		return;

	if (!room)
		return;

	for (int i = room->Passages.Num() - 1; i >= 0; --i)
	{
		LabPassage* passage = room->Passages[i];

		// We remove anything broken
		if (!passage)
		{
			room->Passages.RemoveAt(i);
			continue;
		}

		// TODO shouldn't use try/catch
		//try
		//{
			// We're not interested in normal passages
		if (passage->To && passage->From)
			continue;
		//}
		//catch (...)
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("GOT THAT EXCEPTION WITH PASSAGES"));
		//	room->Passages.RemoveAt(i);
		//	continue;
		//}

		// UE_LOG(LogTemp, Warning, TEXT("Trying to fix passage: x: %d, y: %d"), passage->BotLeftX, passage->BotLeftY);

		bool absolutelyUndeleteable = (room == PlayerRoom || room == ActualPlayerRoom);// && room->Passages.Num() <= 1;
		bool canNotDelete = absolutelyUndeleteable || IsPassageIlluminated(passage);
		if (canNotDelete || !RandBool(DeletePassageToFixProbability))
		{
			// We try to keep passage, though we may still have to delete it

			FRectSpaceStruct minRoomSpace = CreateMinimumRoomSpace(room, passage);	
			// UE_LOG(LogTemp, Warning, TEXT("> Create a room: x: %d, y: %d, sX: %d, sY: %d"), minRoomSpace.BotLeftX, minRoomSpace.BotLeftY, minRoomSpace.SizeX, minRoomSpace.SizeY);


			LabRoom* intersected;
			// Intersects something spawned
			if (MapSpaceIsFree(false, true, minRoomSpace, intersected))
			{
				// Intersects something allocated
				if (MapSpaceIsFree(true, false, minRoomSpace, intersected))
				{
					// We create new room from min space, it also allocates room's space
					LabRoom* newRoom = CreateRandomRoom(minRoomSpace, true, !passage->To ? passage->GridDirection : GetReverseDirection(passage->GridDirection));
					if (newRoom)
					{
						// We add passage to the room
						newRoom->AddPassage(passage);
						continue;
					}
					// else delete
				}
				else
				{
					// We found something that intersects roomSpace but is not spawned yet
					// We check if instead of creating new room (which we can't do since we intersected) we can connect to this room instead

					// Other room should include the room space
					if (IsInside(minRoomSpace, intersected))
					{
						// At this point other room should be considered good
						intersected->AddPassage(passage);
						continue;
					}					
					else if (canNotDelete)
					{
						// UE_LOG(LogTemp, Warning, TEXT("> Can not delete"));

						// TODO
						// if (TryEnlargeRoomToIncludeSpace(intersected, minRoomSpace)
						// {
						//   intersected->AddPassage(passage);
						//   continue;
						// }
						// else
						// {

						// We may delete smth and we will have new rooms to fix
						TArray<LabRoom*> toFix;
						do
						{
							// If intersected can be deleted
							if (!(intersected == PlayerRoom || intersected == ActualPlayerRoom)) // && intersected->Passages.Num() <= 1))
							{
								bool noImportantPassages = true;
								if (!absolutelyUndeleteable)
								{
									for (LabPassage* interPassage : intersected->Passages)
									{
										if (IsPassageIlluminated(interPassage))
										{
											noImportantPassages = false;
											break;
										}
									}
								}

								if (noImportantPassages || absolutelyUndeleteable)
								{
									// UE_LOG(LogTemp, Warning, TEXT("> Forced refix"));

									for (LabPassage* interPassage : intersected->Passages)
									{
										if (interPassage->To && interPassage->To != intersected)
											toFix.Add(interPassage->To);
										if (interPassage->From && interPassage->From != intersected)
											toFix.Add(interPassage->From);
									}
									// toFix.Remove(room);
									PoolRoom(intersected);

									// We should exit the do/while cycle at this point but sometimes we don't, cause we may still intersect with something else
								}
								else break;
							}
							else break;
						} 
						while (!MapSpaceIsFree(true, false, minRoomSpace, intersected));

						// If we deleted some room(s) and now space is free
						if (MapSpaceIsFree(true, false, minRoomSpace, intersected))
						{
							// We create new room from min space
							LabRoom* newRoom = CreateRandomRoom(minRoomSpace, true, !passage->To ? passage->GridDirection : GetReverseDirection(passage->GridDirection));
							if (newRoom)
								newRoom->AddPassage(passage);
							for (LabRoom* roomToFix : toFix)
								FixRoom(roomToFix, depth + 1);
							if (newRoom)
								continue;
							// else delete
						}
						// else delete
					}
					// else delete
				}
			}
			else
			{
				// We found something that intersects roomSpace and is spawned
				// We check if instead of creating new room (which we can't do since we intersected) we can connect to this room instead

				// Room shouldn't be illuminated
				if (PlayerRoom != intersected && ActualPlayerRoom != intersected && !IsRoomIlluminated(intersected))
				{
					// Other room should include the room space
					if (IsInside(minRoomSpace, intersected))
					{
						// Despawn that room so it can be respawned later
						DespawnRoom(intersected);

						// At this point other room should be considered good
						intersected->AddPassage(passage);
						continue;
					}
					// TODO add same as in intersection above?
					// else delete
				}				
				// else delete
			}
		}
		// else delete

		// TODO check if this doesn't break ways into unknown
		// TODO check if at least one connection exists

		room->Passages.RemoveAt(i);
		// We pool and delete passage and spawn a wall instead
		if (SpawnedRoomObjects.Contains(room))
		{
			PoolPassage(passage);
			SpawnBasicWall(passage->BotLeftX, passage->BotLeftY, passage->GridDirection == EDirectionEnum::VE_Up || passage->GridDirection == EDirectionEnum::VE_Down ? passage->Width : 1, passage->GridDirection == EDirectionEnum::VE_Up || passage->GridDirection == EDirectionEnum::VE_Down ? 1 : passage->Width, room);
		}
		delete passage;
	}
}

// Creates random space in the room with specified size for a future object in the room (not world location but offset)
// Returns false if couldn't create
bool AMainGameMode::CreateRandomInsideSpaceOfSize(LabRoom * room, int& xOffset, int& yOffset, const int sizeX, const int sizeY, const bool canBeTaken)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::CreateRandomInsideSpaceOfSize"));

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
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::CreateRandomInsideSpaceOfWidthNearWall"));

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
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::FillRoom"));

	TArray<AActor*> spawnedActors;

	bool isExitRoom = room && room->Passages.Num() == 1 && room->Passages[0]->bIsDoor && room->Passages[0]->Width == ExitDoorWidth;

	if (!isExitRoom)
	{
		// Used for lamps and for doorcards
		FLinearColor color;
		bool colorIsDetermined = false;
		// In rooms with only one door that also isn't white card is almost always given and it has next level, also lamp is almost always spawned
		if (room->Passages.Num() == 1 && room->Passages[0]->bIsDoor && room->Passages[0]->Color != FLinearColor::White)
		{
			colorIsDetermined = true;

			FLinearColor roomColor = room->Passages[0]->Color;
			if (roomColor == FLinearColor::FromSRGBColor(FColor(30, 144, 239)))
				color = FLinearColor::Green;
			else if (roomColor == FLinearColor::Green)
				color = FLinearColor::Yellow;
			else if (roomColor == FLinearColor::Yellow)
				color = FLinearColor::Red;
			else if (roomColor == FLinearColor::Red)
				color = FLinearColor::Black;
			else
				colorIsDetermined = false;
		}

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
				if (!colorIsDetermined)
				{
					color = RandColor();
					while (color == FLinearColor::Black && spawnedActors.Num() < minNumOfLampsOverride)
						color = RandColor();
				}
				AWallLamp* lamp = SpawnWallLamp(room->BotLeftX + xOff, room->BotLeftY + yOff, direction, color, width, room);
				spawnedActors.Add(lamp);
			}
		}

		// Creates a doorcard
		bool shouldSpawnDoorcard = colorIsDetermined || RandBool(SpawnDoorcardProbability);
		for (int i = 0; shouldSpawnDoorcard && i < MaxGenericSpawnTries; ++i)
		{
			int xOff;
			int yOff;
			if (CreateRandomInsideSpaceOfSize(room, xOff, yOff, 1, 1, true))
			{
				// In random rooms cards are almost always blue
				if (!colorIsDetermined)
				{
					// color = FLinearColor::FromSRGBColor(FColor(30, 144, 239));

					color = RandColor();
					while (color == FLinearColor::White || (color == FLinearColor::Black && VisitedOverall < MinVisitedBeforeBlackDoorcardCanSpawn))
						color = RandColor();
				}
				EDirectionEnum direction = RandDirection();
				ADoorcard* doorcard = SpawnDoorcard(room->BotLeftX + xOff, room->BotLeftY + yOff, direction, color, room);
				spawnedActors.Add(doorcard);
				break; // We only spawn once
			}
		}

		// Creates a flashlight
		bool shouldSpawnFlashlight = RandBool(SpawnFlashlightProbability);
		for (int i = 0; shouldSpawnFlashlight && i < MaxGenericSpawnTries; ++i)
		{
			int xOff;
			int yOff;
			if (CreateRandomInsideSpaceOfSize(room, xOff, yOff, 1, 1, false))
			{
				EDirectionEnum direction = RandDirection();
				AFlashlight* flashlight = SpawnFlashlight(room->BotLeftX + xOff, room->BotLeftY + yOff, direction, room);
				spawnedActors.Add(flashlight);
				break; // We only spawn once
			}
		}
	}
	else
	{
		LabPassage* passage = room->Passages[0];
		int x = passage->BotLeftX;
		int y = passage->BotLeftY;
		if (passage->GridDirection == EDirectionEnum::VE_Right)
			x++;
		else if (passage->GridDirection == EDirectionEnum::VE_Left)
			x--;
		else if (passage->GridDirection == EDirectionEnum::VE_Up)
			y++;
		else if (passage->GridDirection == EDirectionEnum::VE_Down)
			y--;
		AExitVolume* exitVolume = SpawnExitVolume(x, y, GetReverseDirection(passage->GridDirection), room);
	}

	return spawnedActors;
}

// Activates all lamps in a single room
void AMainGameMode::ActivateRoomLamps(LabRoom * room, bool forceAll)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::ActivateRoomLamps"));

	if (!room)
		return;

	if (!SpawnedRoomObjects.Contains(room))
		return;

	// Turn on
	if (!RoomsWithLampsOn.Contains(room))
	{
		bool atLeastOneLamp = false;
		for (TScriptInterface<IDeactivatable> obj : SpawnedRoomObjects[room])
		{
			AWallLamp* lamp = Cast<AWallLamp>(obj->_getUObject());
			if (!lamp)
				continue;
			atLeastOneLamp = true;
			lamp->Execute_ActivateIndirectly(lamp);
		}
		if(atLeastOneLamp)
			RoomsWithLampsOn.Add(room);
	}
	// Turn off
	else
	{
		bool turnedOffOne = false;
		bool atLeastOneLampLeft = false;
		bool turnOffAll = forceAll || RandBool(AllLampsInRoomTurnOffProbability);
		for (TScriptInterface<IDeactivatable> obj : SpawnedRoomObjects[room])
		{
			AWallLamp* lamp = Cast<AWallLamp>(obj->_getUObject());
			if (!lamp)
				continue;
			if (turnOffAll)
			{
				if (lamp->IsOn())
					lamp->Execute_ActivateIndirectly(lamp);
			}
			else if (!turnedOffOne)
			{
				if (lamp->IsOn())
				{
					lamp->Execute_ActivateIndirectly(lamp);
					turnedOffOne = true;
				}
			}
			else
			{
				if (lamp->IsOn())
				{
					atLeastOneLampLeft = true;
					break;
				}
			}
		}
		if (turnOffAll || !atLeastOneLampLeft)
			RoomsWithLampsOn.Remove(room);
	}
}

// Returns true if unexpanded rooms are reachable from here
bool AMainGameMode::CanReachUnexpanded(LabRoom * start, TArray<LabRoom*>& checkedRooms)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::CanReachUnexpanded"));

	if (!start)
		return false;

	checkedRooms.Add(start);

	if (!ExpandedRooms.Contains(start))
		return true;

	AMainCharacter* character = Cast<AMainCharacter>(MainPlayerController->GetCharacter());
	for (LabPassage* passage : start->Passages)
	{
		if (passage->bIsDoor && passage->Color != FLinearColor::White && !character->HasDoorcardOfColor(passage->Color))
			continue;

		if (passage->From != start && !checkedRooms.Contains(passage->From))
			if (CanReachUnexpanded(passage->From, checkedRooms))
				return true;
		if (passage->To != start && !checkedRooms.Contains(passage->To))
			if (CanReachUnexpanded(passage->To, checkedRooms))
				return true;;
	}

	return false;
}
bool AMainGameMode::CanReachUnexpanded(LabRoom * start)
{
	TArray<LabRoom*> checkedRooms;
	return CanReachUnexpanded(start, checkedRooms);
}

// Expands room if it's not spawned yet
// Repeats with all adjasent rooms recursively
void AMainGameMode::ExpandInDepth(LabRoom* start, int depth, LabPassage* fromPassage, bool expandExpanded)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::ExpandInDepth1"));

	if (!start)
		return;

	// If not spawned and not expanded (unless we expand expanded)
	if ((expandExpanded || !ExpandedRooms.Contains(start)) && !SpawnedRoomObjects.Contains(start))
		ExpandRoom(start);

	if (depth <= 1)
		return;
	
	// We need a copy to avoid errors during expansion
	TArray<LabPassage*> passagesCopy = start->Passages;
	for (LabPassage* passage : passagesCopy)
	{
		if (passage == fromPassage)
			continue;
		if (passage->From != start)
			ExpandInDepth(passage->From, depth - 1, passage, expandExpanded);
		if (passage->To != start)
			ExpandInDepth(passage->To, depth - 1, passage, expandExpanded);
	}
}
void AMainGameMode::ExpandInDepth(LabRoom * start, int depth)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::ExpandInDepth2"));
	
	// UE_LOG(LogTemp, Warning, TEXT("Expanding:"));
	// UE_LOG(LogTemp, Warning, TEXT("> Try 1"));

	ExpandInDepth(start, depth, nullptr);
	int expandTries = 2;

	AMainCharacter* character = Cast<AMainCharacter>(MainPlayerController->GetCharacter());
	while (!CanReachUnexpanded(start))
	{
		// Make some doors white
		int maxNumPassagesToMakeWhite = 5; // TODO make constant
		for (LabRoom* room : AllocatedRooms)
		{
			for (LabPassage* pas : room->Passages)
			{
				if (!pas->bIsDoor || pas->Color == FLinearColor::White || pas->Width == ExitDoorWidth || SpawnedPassageObjects.Contains(pas) || character->HasDoorcardOfColor(pas->Color))
					continue;

				pas->Color = FLinearColor::White;
				--maxNumPassagesToMakeWhite;
				if (maxNumPassagesToMakeWhite <= 0)
					break;
			}
			if (maxNumPassagesToMakeWhite <= 0)
				break;
		}
		if (CanReachUnexpanded(start))
			break;

		// Disable nearby lamps
		if (expandTries == MaxExpandTriesBeforeDisablingLights + 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("> !!! Disabled a lot of lamps"));
			
			// TODO should not turn off ALL lamps
			for (int i = RoomsWithLampsOn.Num() - 1; i >= 0; --i)
			{
				LabRoom* room = RoomsWithLampsOn[i];
				ActivateRoomLamps(room, true);
			}
		}

		if (expandTries <= MaxExpandTriesOverall)
		{
			// UE_LOG(LogTemp, Warning, TEXT("> Try %d"), expandTries);

			if (expandTries > MinExpandTriesBeforeReshaping)
				ReshapeAllDarkness(); // We do his to prevend being stuck
			// ExpandInDepth(start, depth + expandTries / 2, nullptr, true);
			ExpandInDepth(start, depth, nullptr, true);

			++expandTries;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("> !!! Failed to get a pass to unexpanded"));
			break;
		}
	}
}
// Spawns and fills room if it's not spawned yet
// Repeats with all adjasent rooms recursively
void AMainGameMode::SpawnFillInDepth(LabRoom * start, int depth, LabPassage* fromPassage, FVector initialPasLoc)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::SpawnFillInDepth1"));

	if (!start)
		return;

	// If not spawned
	if (!SpawnedRoomObjects.Contains(start))
	{
		SpawnRoom(start);
		FillRoom(start);
	}

	if (depth <= 1)
		return;
	
	for (LabPassage* passage : start->Passages)
	{
		if (passage == fromPassage)
			continue;

		// location of the floor
		FVector pasLoc = Cast<AActor>(SpawnedPassageObjects[passage][0]->_getUObject())->GetActorLocation() + FVector(0, 0, 30);

		if (CanSee(initialPasLoc, pasLoc))
		{
			if (passage->From != start)
				SpawnFillInDepth(passage->From, depth - 1, passage, initialPasLoc);
			if (passage->To != start)
				SpawnFillInDepth(passage->To, depth - 1, passage, initialPasLoc);
		}
	}
}
void AMainGameMode::SpawnFillInDepth(LabRoom* start, int depth)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::SpawnFillInDepth2"));

	if (!start)
		return;

	// If not spawned
	if (!SpawnedRoomObjects.Contains(start))
	{
		SpawnRoom(start);
		FillRoom(start);
	}

	if (depth <= 1)
		return;

	for (LabPassage* passage : start->Passages)
	{
		// location of the floor
		FVector initialPasLoc = Cast<AActor>(SpawnedPassageObjects[passage][0]->_getUObject())->GetActorLocation() + FVector(0, 0, 30);
		if ((passage->From == start && passage->GridDirection == EDirectionEnum::VE_Right) || (passage->To == start && passage->GridDirection == EDirectionEnum::VE_Left))
			initialPasLoc += FVector(0, 30, 0);
		else if ((passage->From == start && passage->GridDirection == EDirectionEnum::VE_Left) || (passage->To == start && passage->GridDirection == EDirectionEnum::VE_Right))
			initialPasLoc += FVector(0, -30, 0);
		else if ((passage->From == start && passage->GridDirection == EDirectionEnum::VE_Up) || (passage->To == start && passage->GridDirection == EDirectionEnum::VE_Down))
			initialPasLoc += FVector(30, 0, 0);
		else if ((passage->From == start && passage->GridDirection == EDirectionEnum::VE_Down) || (passage->To == start && passage->GridDirection == EDirectionEnum::VE_Up))
			initialPasLoc += FVector(-30, 0, 0);
		
		if (passage->From != start)
			SpawnFillInDepth(passage->From, depth - 1, passage, initialPasLoc);
		if (passage->To != start)
			SpawnFillInDepth(passage->To, depth - 1, passage, initialPasLoc);
	}
}

// Generates map
void AMainGameMode::GenerateMap()
{
	LabRoom* startRoom = CreateStartRoom();
	ExpandRoom(startRoom, 1);
	SpawnRoom(startRoom);
	FillRoom(startRoom, 1);
	MainPlayerController->GetCharacter()->SetActorLocation(FVector(25, 25, 90)); //, false, nullptr, ETeleportType::TeleportPhysics);
	// GetCharacterRoom();
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
	static ConstructorHelpers::FObjectFinder<UClass> lighterBP(TEXT("Class'/Game/Blueprints/LighterBP.LighterBP_C'"));
	if (lighterBP.Succeeded())
		LighterBP = lighterBP.Object;
	static ConstructorHelpers::FObjectFinder<UClass> doorcardBP(TEXT("Class'/Game/Blueprints/DoorcardBP.DoorcardBP_C'"));
	if (doorcardBP.Succeeded())
		DoorcardBP = doorcardBP.Object;
	static ConstructorHelpers::FObjectFinder<UClass> exitVolumeBP(TEXT("Class'/Game/Blueprints/ExitVolumeBP.ExitVolumeBP_C'"));
	if (exitVolumeBP.Succeeded())
		ExitVolumeBP = exitVolumeBP.Object;

	// Set to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	// TODO remove before shipping
	// ShowHideDebug();

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

	// Then we find the character controller
	APlayerController* controller = gameWorld->GetFirstPlayerController();
	MainPlayerController = Cast<AMainPlayerController>(controller);

	// Finally we generate map
	GenerateMap(); 
	
	// Make world reshape every few seconds even if character doesn't change rooms
	FTimerHandle handler;
	((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &AMainGameMode::CompleteReshapeAllDarknessAroundOnTick, ReshapeDarknessTick, true, ReshapeDarknessTick);

	// TODO delete, used only for tests sometimes
	/*((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &AMainGameMode::ResetMap, 2, true, 2);*/
}

// Called every frame
void AMainGameMode::Tick(const float deltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("MainGameMode::Tick"));

	Super::Tick(deltaTime);
	
	// Updates PlayerRoom, calls OnEnterRoom
	GetCharacterRoom();

	// Turns off some lamps from time to time
	for (int i = RoomsWithLampsOn.Num() - 1; i >= 0; --i)
	{
		LabRoom* room = RoomsWithLampsOn[i];
		if (RandBool(LampsTurnOffPerSecondProbability * deltaTime))
			ActivateRoomLamps(room);
	}

	ACharacter* tempCharacter = MainPlayerController->GetCharacter();
	AMainCharacter* character = tempCharacter ? Cast<AMainCharacter>(tempCharacter) : nullptr;

	// If player finds black card, we make darkness know
	if (character && DarknessController && !DarknessController->bIsPersistent && character->HasDoorcardOfColor(FLinearColor::Black))
		DarknessController->OnPlayerFindsBlackCard();

	// On screen debug
	if (bShowDebug && GEngine)
	{
		// Pools debug
		// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Pools: default: %d, floor: %d, wall: %d, door: %d, lamp: %d, flashlight: %d"), DefaultPool.Num(), BasicFloorPool.Num(), BasicWallPool.Num(), BasicDoorPool.Num(), WallLampPool.Num(), FlashlightPool.Num()), true);

		// Darkness debug
		APawn* tempDarkness = DarknessController->GetPawn();
		if (tempDarkness)
		{
			ADarkness* darkness = Cast<ADarkness>(tempDarkness);

			// Title
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT("Darkness:"), false);
			
			// Current location
			int darkX, darkY;
			FVector darknessLocation = darkness->GetActorLocation();
			WorldToGrid(darknessLocation.X, darknessLocation.Y, darkX, darkY);
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("> Location: X: %d, Y: %d"), darkX, darkY), false);
			
			// Current state
			FString state = "Unknown";
			switch (DarknessController->State)
			{
			case EDarkStateEnum::VE_Passive:
				state = "Passive";
				break;
			case EDarkStateEnum::VE_Hunting:
				state = "Hunting";
				break;
			case EDarkStateEnum::VE_Retreating:
				state = "Retreating";
				break;
			}
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("> State: %s"), *state), false);

			// Current luminosity
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("> Luminosity: %f"), darkness->Luminosity), false);

			// Light resistance
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("> Resistance: %f"), darkness->LightResistance), false);

			// Empty line
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT(""), false);
		}

		// Character debug
		if (character)
		{
			// Title
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT("Character:"), false);

			// Current location
			int charX, charY;
			GetCharacterLocation(charX, charY);
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("> Location: X: %d, Y: %d"), charX, charY), false);

			// Current room
			// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("> Room: X: %d, Y: %d, sX: %d, sY: %d"), PlayerRoom->BotLeftX, PlayerRoom->BotLeftY, PlayerRoom->SizeX, PlayerRoom->SizeY), false);

			// Light in current room
			// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("> room luminosity: %f"), GetRoomLightingAmount(PlayerRoom)), true);

			// Number of "lives" left
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("> Lives: %d"), MainPlayerController->Lives), false);

			// Number of visited rooms
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("> Visited rooms: %d"), VisitedOverall), false);

			// Doorcards
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("> Doorcards: %s%s%s%s%s"),
				character->HasDoorcardOfColor(FLinearColor::FromSRGBColor(FColor(30, 144, 239))) ? TEXT("Blue ") : TEXT(""),
				character->HasDoorcardOfColor(FLinearColor::Green) ? TEXT("Green ") : TEXT(""),
				character->HasDoorcardOfColor(FLinearColor::Yellow) ? TEXT("Yellow ") : TEXT(""),
				character->HasDoorcardOfColor(FLinearColor::Red) ? TEXT("Red ") : TEXT(""),
				character->HasDoorcardOfColor(FLinearColor::Black) ? TEXT("Black") : TEXT("")), false);

			// Activatable and equipped
			TArray<IInformative*> informativeObjects;
			TArray<FString> informativeNames;
			// Get activatable
			TScriptInterface<IActivatable> activatable = character->GetActivatable();
			informativeObjects.Add(activatable ? Cast<IInformative>(activatable->_getUObject()) : nullptr);
			informativeNames.Add("> Activatable");
			// Get equipped
			informativeObjects.Add(character->EquipedObject ? Cast<IInformative>(character->EquipedObject->_getUObject()) : nullptr);
			informativeNames.Add("> Equipped");
			// Finally print
			for (int i = 0; i < informativeObjects.Num(); ++i)
			{
				IInformative* informative = informativeObjects[i];
				FString name = informativeNames[i];

				if (informative)
				{
					GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("%s: %s"), *name, *(informative->Execute_GetName(informative->_getUObject())).ToString()), false);
					GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("   > %s"), *(informative->Execute_GetBasicInfo(informative->_getUObject())).ToString()), false);
				}
				else
					GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("%s: %s"), *name, TEXT("None")), false);
			}
			/*if (character->LighterIndex >= 0)
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Lighter gas: %f"), Cast<ALighter>(character->Inventory[character->LighterIndex]->_getUObject())->PowerLevel * 100), false);*/
			if (character->FlashlightIndex >= 0)
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Flashlight power: %f"), Cast<AFlashlight>(character->Inventory[character->FlashlightIndex]->_getUObject())->PowerLevel * 100), false);
			

			// Empty line
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT(""), false);
		}
	}
}

// Called when actor is being removed from the play
void AMainGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UE_LOG(LogTemp, Warning, TEXT("EndPlay called"));

	// Clear all saved rooms
	TArray<LabRoom*> allRooms;
	AllocatedRoomSpace.GetKeys(allRooms);
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