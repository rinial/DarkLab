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
	// Returns true with certain probability
	bool RandBool(const float probability);
	// Returns random color with certain probabilities
	FLinearColor RandColor();
	// Returns random direction 
	EDirectionEnum RandDirection();

	// Returns the light level and the location of the brightest light
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const bool sixPoints = false, const float sixPointsRadius = 30.0f, const bool fourMore = false, const bool debug = false);
	float GetLightingAmount(FVector& lightLoc, const FVector location, const bool sixPoints = false, const float sixPointsRadius = 30.0f, const bool fourMore = false, const bool debug = false);
	float GetLightingAmount(FVector& lightLoc, const TArray<FVector> locations, const bool debug = false);
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const FVector location, const bool sixPoints = false, const float sixPointsRadius = 30.0f, const bool fourMore = false, const bool debug = false);
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const TArray<FVector> locations, const bool debug = false);
	// Returns true if one actor/location can see other actor/location
	// Its not about visibility to human eye, doesn't take light into account
	bool CanSee(const AActor* actor1, const AActor* actor2, const bool debug = false);
	bool CanSee(const FVector location, const AActor* actor, const bool debug = false);
	bool CanSee(const AActor* actor, const FVector location, const bool debug = false);
	bool CanSee(const FVector location1, const FVector location2, const bool debug = false);
	bool CanSee(const AActor* actor1, const FVector location1, const AActor* actor2, const bool debug = false);
	bool CanSee(const AActor* actor1, const AActor* actor2, const FVector location2, const bool debug = false);
	bool CanSee(const AActor* actor1, const FVector location1, const FVector location2, const bool debug = false);
	bool CanSee(const FVector location1, const AActor* actor2, const FVector location2, const bool debug = false);
	bool CanSee(const AActor* actor1, const FVector location1, const AActor* actor2, const FVector location2, const bool debug = false);

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
	void PoolMap();

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

	// Room is allocated and can't be allocated again
	void AllocateRoom(LabRoom* room);
	// Room is not allocated anymore
	void DeallocateRoom(LabRoom* room);
	// Space in the room is allocated and can't be allocated again
	void AllocateRoomSpace(LabRoom* room, FRectSpaceStruct space, bool local = true);
	void AllocateRoomSpace(LabRoom* room, const int xOffset, const int yOffset, const EDirectionEnum direction, const int width, bool local = true);
	void AllocateRoomSpace(LabRoom* room, const int xOffset, const int yOffset, const int sizeX, const int sizeY, bool local = true);
	// Space in the room is not allocated anymore
	void DeallocateRoomSpace(LabRoom* room, FRectSpaceStruct space);

	// Returns true if there is free rectangular space
	// Returns another room that intersected the sent space
	bool MapSpaceIsFree(bool amongAllocated, bool amongSpawned, FRectSpaceStruct space);
	bool MapSpaceIsFree(bool amongAllocated, bool amongSpawned, const int botLeftX, const int botLeftY, const int sizeX = 1, const int sizeY = 1);
	bool MapSpaceIsFree(bool amongAllocated, bool amongSpawned, FRectSpaceStruct space, LabRoom*& intersected);
	bool MapSpaceIsFree(bool amongAllocated, bool amongSpawned, const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabRoom*& intersected);

	// Returns true if there is free rectangular space in a room
	bool RoomSpaceIsFree(LabRoom* room, FRectSpaceStruct space, const bool forPassage = false, const bool forDoor = false);
	bool RoomSpaceIsFree(LabRoom* room, const int xOffset, const int yOffset, EDirectionEnum direction, const int width = 4, const bool forPassage = false, const bool forDoor = false);
	bool RoomSpaceIsFree(LabRoom* room, const int xOffset, const int yOffset, const int sizeX = 1, const int sizeY = 1, const bool forPassage = false, const bool forDoor = false);

	// Returns true is one intersects the other (more than just side)
	bool Intersect(LabRoom* room1, LabRoom* room2);
	bool Intersect(FRectSpaceStruct space1, LabRoom* room2);
	bool Intersect(LabRoom* room1, FRectSpaceStruct space2);
	bool Intersect(FRectSpaceStruct space1, FRectSpaceStruct space2);

	// Returns true is first is inside second
	bool IsInside(LabRoom* room1, LabRoom* room2);
	bool IsInside(FRectSpaceStruct space1, LabRoom* room2);
	bool IsInside(LabRoom* room1, FRectSpaceStruct space2);
	bool IsInside(FRectSpaceStruct space1, FRectSpaceStruct space2);

	// Tries to create a room and allocate space for it
	LabRoom* CreateRoom(FRectSpaceStruct space);
	LabRoom* CreateRoom(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY);

	// Creates starting room
	LabRoom* CreateStartRoom();

	// Creates random space for a future passage (not world location but offsets)
	// Doesn't take other passages into account. Direction is always out
	FRectSpaceStruct CreateRandomPassageSpace(LabRoom* room, EDirectionEnum& direction, const bool forDoor = false);

	// Creates minimum space for a room near passage for tests and allocation
	// TODO maybe it should take room size just in case other room gets destroyed
	FRectSpaceStruct CreateMinimumRoomSpace(LabRoom* room, FRectSpaceStruct passageSpace, EDirectionEnum direction);
	// Creates random room space based on minimum room space
	FRectSpaceStruct CreateRandomRoomSpace(FRectSpaceStruct minSpace, bool fromPassage = false, EDirectionEnum direction = EDirectionEnum::VE_Up);

	// Shrinks space to not include specified room but still include minSpace
	// If prioritize horizontal is true, it tries to shrink horizontal part, otherwise vertival
	void ShrinkSpace(FRectSpaceStruct& currentSpace, FRectSpaceStruct minSpace, LabRoom* toAvoid, bool prioritizeHorizontal);

	// Creates a random room based on minimum room space
	LabRoom* CreateRandomRoom(FRectSpaceStruct minSpace, bool fromPassage = false, EDirectionEnum direction = EDirectionEnum::VE_Up);

	// Creates and adds a random passage to the room, returns passage or nullptr and returns allocated room space by reference or another room that is now connected
	LabPassage* CreateAndAddRandomPassage(LabRoom* room, FRectSpaceStruct& roomSpace, LabRoom*& possibleRoomConnection);

	// Creates new passages in the room
	// Create new rooms for passages 
	// Returns new rooms
	TArray<LabRoom*> ExpandRoom(LabRoom* room);

	// Creates random space in the room with specified size for a future object in the room (not world location but offset)
	// Returns false if couldn't create
	bool CreateRandomInsideSpaceOfSize(LabRoom* room, int& xOffset, int& yOffset, const int sizeX, const int sizeY, const bool canBeTaken = false);
	// Same but near wall and returns direction from wall (width is along wall)
	bool CreateRandomInsideSpaceOfWidthNearWall(LabRoom* room, int& xOffset, int& yOffset, const int width, EDirectionEnum& direction, const bool canBeTaken = false);

	// Fills room with random objects, spawns and returns them
	// Should always be called on a room that is already spawned
	TArray<AActor*> FillRoom(LabRoom* room, int minNumOfLampsOverride = 0);

public:
	// Generates map
	void GenerateMap();
	// Resets the map
	void ResetMap();
	// Shows/hides debug
	void ShowHideDebug();

protected:
	// For debug
	bool bShowDebug = false;

	// Rooms that are created but are not spawned yet and can still be changed
	TArray<LabRoom*> AllocatedRooms;

	// Room-specific space taken by various objects (not world locations but offsets)
	TMap<LabRoom*, TArray<FRectSpaceStruct>> AllocatedRoomSpace;

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

	// Constants used for generation
	static const int MinRoomSize = 4; // Can't be lower than 4
	static const int MaxRoomSize = 50;
	static const int MaxRoomArea = 300;
	static const int MinRoomNumOfPassages = 1; // Can't be lower than 1
	static const int MaxRoomNumOfPassages = 10; 
	static const int MaxRoomPassageCreationTriesPerDesired = 2; 
	static const int MinPassageWidth = 2; // Can't be lower than 2
	static const int MaxPassageWidth = 10; 
	static const int NormalDoorWidth = 4; // Can't be lower than 2
	static const int BigDoorWidth = 6;
	static const int MinDistanceBetweenPassages = 1; // Can't be lower than 1
	static const int MinDistanceInsideToPassage = 1; // Maybe it should be 2
	static const int MinRoomNumOfLamps = 0; 
	static const int MaxRoomNumOfLampsPerHundredArea = 2;
	static const int MaxRoomLampCreationTriesPerDesired = 2;
	static const int MinLampWidth = 1;
	static const int MaxLampWidth = 2;
	static const int MaxGenericSpawnTries = 3;
	// Probabilities
	static const float ConnectToOtherRoomProbability;
	static const float PassageIsDoorProbability;
	static const float DoorIsNormalProbability;
	static const float SpawnFlashlightProbability;
	static const float BlueProbability;
	static const float GreenProbability;
	static const float YellowProbability;
	static const float RedProbability;
	static const float BlackProbability;

	// Pointers to existing controllers
	class ADarknessController* DarknessController;
	
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

	// TODO delete, used just for tests
	float TimeSinceLastGeneration = 100.0f;

public:
	// Called every frame
	virtual void Tick(const float deltaTime) override;

protected:
	// Called when actor is being removed from the play
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// TODO delete?
	// Called at start of seamless travel, or right before map change for hard travel
	virtual void StartToLeaveMap() override;
};