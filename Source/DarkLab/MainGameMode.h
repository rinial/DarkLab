// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Placeable.h"
#include "MainGameMode.generated.h"

class IDeactivatable;
class ABasicFloor;
class ABasicWall;
class ABasicDoor;
class AWallLamp;
class AFlashlight;
class LabRoom;
class LabPassage;

// Controls the game
UCLASS(Blueprintable)
class DARKLAB_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Returns the light level and the location of the brightest light
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const bool sixPoints = false, const float sixPointsRadius = 30.0f, const bool fourMore = false);
	float GetLightingAmount(FVector& lightLoc, const FVector location, const bool sixPoints = false, const float sixPointsRadius = 30.0f, const bool fourMore = false);
	float GetLightingAmount(FVector& lightLoc, const TArray<FVector> locations);
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const FVector location, const bool sixPoints = false, const float sixPointsRadius = 30.0f, const bool fourMore = false);
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const TArray<FVector> locations);

protected:
	// Places an object on the map
	// TODO return false if can't place?
	void PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftX, const int botLeftY, const EDirectionEnum direction = EDirectionEnum::VE_Up);
	void PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftX, const int botLeftY, const int sizeX, const int sizeY = 1);
	void PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, const int sizeZ);
	void PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftX, const int botLeftY, const EDirectionEnum direction, const int sizeX, const int sizeY = 1);
	void PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftX, const int botLeftY, const EDirectionEnum direction, const int sizeX, const int sizeY, const int sizeZ);
	void PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc = FIntVector(0, 0, 0), const EDirectionEnum direction = EDirectionEnum::VE_Up);
	void PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc, const int sizeX, const int sizeY = 1);
	void PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc, const int sizeX, const int sizeY, const int sizeZ);
	void PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc, const EDirectionEnum direction, const int sizeX, const int sizeY = 1);
	void PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc, const EDirectionEnum direction, const int sizeX, const int sizeY, const int sizeZ);

	// Gets the pool for the object/class
	TArray<TScriptInterface<IDeactivatable>>& GetCorrectPool(TScriptInterface<IDeactivatable> object);
	TArray<TScriptInterface<IDeactivatable>>& GetCorrectPool(UClass* cl);

	// Deactivates and adds to a pool
	UFUNCTION(BlueprintCallable, Category = "Pools")
	void PoolObject(TScriptInterface<IDeactivatable> object);
	UFUNCTION(BlueprintCallable, Category = "Pools")
	void PoolObjects(TArray<TScriptInterface<IDeactivatable>>& objects);

	// Pool full parts of the lab
	void PoolRoom(LabRoom* room);
	void PoolPassage(LabPassage* passage);

	// Tries to find a poolable object in a specified array
	UFUNCTION(BlueprintCallable, Category = "Pools")
	UObject* TryGetPoolable(UClass* cl);

	// Spawn specific objects
	ABasicFloor* SpawnBasicFloor(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabRoom* room = nullptr);
	ABasicFloor* SpawnBasicFloor(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabPassage* passage);
	ABasicWall* SpawnBasicWall(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabRoom* room = nullptr);
	ABasicDoor* SpawnBasicDoor(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color = FLinearColor::White, const int width = 4, LabPassage* passage = nullptr);
	AWallLamp* SpawnWallLamp(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color = FLinearColor::White, const int width = 1, LabRoom* room = nullptr);
	AFlashlight* SpawnFlashlight(const int botLeftX, const int botLeftY, const EDirectionEnum direction = EDirectionEnum::VE_Up);

	// Spawn full parts of the lab
	void SpawnRoom(LabRoom* room);
	void SpawnPassage(LabPassage* passage, LabRoom* room = nullptr);

	// Space is allocated and can't be allocated again
	FRectSpaceStruct* AllocateSpace(LabRoom* room);
	FRectSpaceStruct* AllocateSpace(int botLeftX, int botLeftY, int sizeX = 1, int sizeY = 1);
	FRectSpaceStruct* AllocateSpace(FRectSpaceStruct& space);
	// Space is not allocated anymore
	void DeallocateSpace(FRectSpaceStruct& space);
	void DeallocateSpace(LabRoom* room);
	void DeallocateSpace(int botLeftX, int botLeftY, int sizeX = 1, int sizeY = 1);

	// Returns true if there is free rectangular space
	bool RectSpaceIsFree(FRectSpaceStruct& space);
	bool RectSpaceIsFree(int botLeftX, int botLeftY, int sizeX = 1, int sizeY = 1);

	// Tries to create a room and allocate space for it
	LabRoom* CreateRoom(int botLeftX, int botLeftY, int sizeX, int sizeY);
	LabRoom* CreateRoom(FRectSpaceStruct& space);

protected:
	// Constants used for generation
	static const int MinRoomSize = 4; // Can't be lower than 4
	static const int MaxRoomSize = 50;
	static const int MaxRoomArea = 300;
	static const int MinPassageWidth = 2; // Can't be lower than 2
	static const int MaxPassageWidth = 8; 

	// All space in use
	TArray<FRectSpaceStruct> AllocatedSpace;
	TMap<LabRoom*, FRectSpaceStruct*> AllocatedRoomSpace;
	//// Left and right (bottom and top) sorted coordinates of rectangles
	//TArray<int> AllocatedX1Indices;
	//TArray<int> AllocatedX2Indices;
	//TArray<int> AllocatedY1Indices;
	//TArray<int> AllocatedY2Indices;
	//// Indices that are not used anymore and can be used again
	//// We don't just delete from AllocatedSpace, cause we would have to decrement a lot of indices in indices arrays
	//TArray<int> DeallocatedIndices;

	// Spawned map parts
	// Does not include pickupable objects
	TMap<LabRoom*, TArray<TScriptInterface<IDeactivatable>>> SpawnedRoomObjects;
	TMap<LabPassage*, TArray<TScriptInterface<IDeactivatable>>> SpawnedPassageObjects;

	// Pools
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pools")
	TArray<TScriptInterface<IDeactivatable>> DefaultPool;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pools")
	TArray<TScriptInterface<IDeactivatable>> BasicFloorPool;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pools")
	TArray<TScriptInterface<IDeactivatable>> BasicWallPool;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pools")
	TArray<TScriptInterface<IDeactivatable>> BasicDoorPool;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pools")
	TArray<TScriptInterface<IDeactivatable>> WallLampPool;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pools")
	TArray<TScriptInterface<IDeactivatable>> FlashlightPool;
	
private:
	// Classes used for spawning
	TSubclassOf<ABasicFloor> BasicFloorBP;
	TSubclassOf<ABasicWall> BasicWallBP;
	TSubclassOf<ABasicDoor> BasicDoorBP;
	TSubclassOf<AWallLamp> WallLampBP;
	TSubclassOf<AFlashlight> FlashlightBP;

public:
	// Sets default values
	AMainGameMode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called when actor is being removed from the play
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// Called at start of seamless travel, or right before map change for hard travel
	virtual void StartToLeaveMap() override;
};