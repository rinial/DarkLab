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
class ALighter;
class ADoorcard;
class AExitVolume;
class LabRoom;
class LabPassage;

// Controls the game
UCLASS(Blueprintable)
class DARKLAB_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Returns true with certain probability
	UFUNCTION(BlueprintCallable, Category = "Generic functions")
	bool RandBool(const float probability);
	// Returns random color with certain probabilities
	UFUNCTION(BlueprintCallable, Category = "Generic functions")
	FLinearColor RandColor();
	// Returns random direction 
	UFUNCTION(BlueprintCallable, Category = "Generic functions")
	EDirectionEnum RandDirection();

	// Returns the light level and the location of the brightest light
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const bool sixPoints = false, const float sixPointsRadius = 30.0f, const bool fourMore = false, const bool returnFirstPositive = false);
	float GetLightingAmount(FVector& lightLoc, const FVector location, const bool sixPoints = false, const float sixPointsRadius = 30.0f, const bool fourMore = false, const bool returnFirstPositive = false);
	float GetLightingAmount(FVector& lightLoc, const TArray<FVector> locations, const bool returnFirstPositive = false);
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const FVector location, const bool sixPoints = false, const float sixPointsRadius = 30.0f, const bool fourMore = false, const bool returnFirstPositive = false);
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const TArray<FVector> locations, const bool returnFirstPositive = false);
	// Returns true if one actor/location can see other actor/location
	// Its not about visibility to human eye, doesn't take light into account
	bool CanSee(const AActor* actor1, const AActor* actor2);
	bool CanSee(const FVector location, const AActor* actor);
	bool CanSee(const AActor* actor, const FVector location);
	bool CanSee(const FVector location1, const FVector location2);
	bool CanSee(const AActor* actor1, const FVector location1, const AActor* actor2);
	bool CanSee(const AActor* actor1, const AActor* actor2, const FVector location2);
	bool CanSee(const AActor* actor1, const FVector location1, const FVector location2);
	bool CanSee(const FVector location1, const AActor* actor2, const FVector location2);
	bool CanSee(const AActor* actor1, const FVector location1, const AActor* actor2, const FVector location2);

protected:
	// Returns the light level for a passage
	float GetPassageLightingAmount(LabPassage* passage, bool oneSide = false, bool innerSide = true, const bool returnFirstPositive = false);
	// Returns true if passage is illuminated
	bool IsPassageIlluminated(LabPassage* passage, bool oneSide = false, bool innerSide = true);
	// Returns the light level for a room
	float GetRoomLightingAmount(LabRoom* room, const bool returnFirstPositive = false);
	// Returns true if the room is in light
	bool IsRoomIlluminated(LabRoom* room);

public:
	// Changes world location into grid location
	static void WorldToGrid(const float worldX, const float worldY, int& gridX, int& gridY);
	// Changes grid location into world location
	static void GridToWorld(const int gridX, const int gridY, float& worldX, float& worldY);
	static void GridToWorld(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, float& worldX, float& worldY);

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

	// Returns by reference character's location on the grid
	UFUNCTION(BlueprintCallable, Category = "Generic functions")
	void GetCharacterLocation(int& x, int& y);
	// Returns the room the character is in
	LabRoom* GetCharacterRoom();
	// Called when character enters new room
	void OnEnterRoom(); // LabRoom* lastRoom, LabRoom* newRoom);

public:
	// Called when character loses all of his lives
	void OnLoss();
	// Called when character is enabled to reset the map
	void OnCharacterEnabled();
	// Called when character reached the exit
	void OnExitReached();

	// Called when character picks up an object to delete it from arrays without pooling
	void OnPickUp(TScriptInterface<class IPickupable> object);
	// Called when exit door is opened to
	void OnExitOpened(class ABasicDoor* door);

protected:
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
	UFUNCTION(BlueprintCallable, Category = "Pools")
	void PoolMap();

	// Pools dark area returning all rooms that now need fixing
	void PoolDarkness(LabRoom* start, int depth, TArray<LabRoom*>& toFix, bool stopAtFirstIfLit = true);
	void PoolDarkness(LabRoom* start, int depth, TArray<LabRoom*>& toFix, TArray<LabRoom*>& toPool, bool stopAtFirstIfLit = true);

	// Pools dark area and fixes every room that needs fixing
	void ReshapeDarkness(LabRoom* start, int depth, bool stopAtFirstIfLit = true);
	// Reshapes darkness and expands, spawns and fills rooms
	void CompleteReshapeDarkness(LabRoom* start, bool stopAtFirstIfLit = true);
	// Reshapes darkness in player room
	UFUNCTION(BlueprintCallable, Category = "Reshape")
	void CompleteReshapeDarknessAround();
	// Pools all dark rooms on the map and fixes every room that needs fixing
	UFUNCTION(BlueprintCallable, Category = "Reshape")
	void ReshapeAllDarkness();
	// Reshapes all darkness and also expands spawns and fills around player's room
	UFUNCTION(BlueprintCallable, Category = "Reshape")
	void CompleteReshapeAllDarknessAround();
	// Calls CompleteReshapeAllDarknessAround with specified probability
	UFUNCTION(BlueprintCallable, Category = "Reshape")
	void CompleteReshapeAllDarknessAroundOnTick();

	// Tries to find a poolable object in a specified array
	UFUNCTION(BlueprintCallable, Category = "Pools")
	UObject* TryGetPoolable(UClass* cl);

public:
	// Spawn specific objects
	ABasicFloor* SpawnBasicFloor(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabRoom* room = nullptr);
	ABasicFloor* SpawnBasicFloor(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabPassage* passage);
	ABasicWall* SpawnBasicWall(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY, LabRoom* room = nullptr);
	ABasicDoor* SpawnBasicDoor(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color = FLinearColor::White, const int width = 4, LabPassage* passage = nullptr);
	AWallLamp* SpawnWallLamp(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color = FLinearColor::White, const int width = 1, LabRoom* room = nullptr);
	AFlashlight* SpawnFlashlight(const int botLeftX, const int botLeftY, const EDirectionEnum direction, LabRoom* room = nullptr);
	ALighter* SpawnLighter(const int botLeftX, const int botLeftY, const EDirectionEnum direction = EDirectionEnum::VE_Up, LabRoom* room = nullptr);
	ADoorcard* SpawnDoorcard(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color, LabRoom* room = nullptr);
	AExitVolume* SpawnExitVolume(const int botLeftX, const int botLeftY, const EDirectionEnum direction, LabRoom* room = nullptr);

protected:
	// Spawn full parts of the lab
	void SpawnRoom(LabRoom* room);
	void SpawnPassage(LabPassage* passage, LabRoom* room = nullptr);
	// Despawns room so it can be respawned later
	void DespawnRoom(LabRoom* room);

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

	// Reverses direction
	UFUNCTION(BlueprintCallable, Category = "Generic functions")
	EDirectionEnum GetReverseDirection(EDirectionEnum direction);

	// Creates random space for a future passage (not world location but offsets)
	// Doesn't take other passages into account. Direction is always out
	FRectSpaceStruct CreateRandomPassageSpace(LabRoom* room, EDirectionEnum& direction, const bool forDoor = false);
	// Creates a passage space for existing passage
	FRectSpaceStruct CreatePassageSpaceFromPassage(LabRoom* room, LabPassage* passage);

	// Creates minimum space for a room near passage space for tests and allocation
	// TODO maybe it should take room size just in case other room gets destroyed
	FRectSpaceStruct CreateMinimumRoomSpace(LabRoom* room, FRectSpaceStruct passageSpace, EDirectionEnum direction, bool widerForDoor = false);
	// Creates minimum space for a room near passage for tests and allocation
	FRectSpaceStruct CreateMinimumRoomSpace(LabRoom* room, LabPassage* passage);

	// Creates random room space based on minimum room space
	FRectSpaceStruct CreateRandomRoomSpace(FRectSpaceStruct minSpace, bool fromPassage = false, EDirectionEnum direction = EDirectionEnum::VE_Up);

	// Shrinks space to not include specified room but still include minSpace
	// If prioritize horizontal is true, it tries to shrink horizontal part, otherwise vertival
	// Returns true if manages to shrink
	bool ShrinkSpace(FRectSpaceStruct& currentSpace, FRectSpaceStruct minSpace, LabRoom* toAvoid, bool prioritizeX);
	bool TryShrinkX(FRectSpaceStruct& currentSpace, FRectSpaceStruct minSpace, LabRoom* toAvoid);
	bool TryShrinkY(FRectSpaceStruct& currentSpace, FRectSpaceStruct minSpace, LabRoom* toAvoid);

	// Creates a random room based on minimum room space
	LabRoom* CreateRandomRoom(FRectSpaceStruct minSpace, bool fromPassage = false, EDirectionEnum direction = EDirectionEnum::VE_Up, bool keepMinimum = false);

	// Creates and adds a random passage to the room, returns passage or nullptr and returns allocated room space by reference or another room that is now connected
	LabPassage* CreateAndAddRandomPassage(LabRoom* room, FRectSpaceStruct& roomSpace, LabRoom*& possibleRoomConnection);

	// Creates new passages in the room
	// Create new rooms for passages 
	// Returns new rooms
	TArray<LabRoom*> ExpandRoom(LabRoom* room, int desiredNumOfPassagesOverride = 0);

	// Fixes room's passages that lead nowhere, creating a room for them or deleting them
	// Also spawns a wall over previous passage if room was spawned
	void FixRoom(LabRoom* room, int depth = 1);

	// Creates random space in the room with specified size for a future object in the room (not world location but offset)
	// Returns false if couldn't create
	bool CreateRandomInsideSpaceOfSize(LabRoom* room, int& xOffset, int& yOffset, const int sizeX, const int sizeY, const bool canBeTaken = false);
	// Same but near wall and returns direction from wall (width is along wall)
	bool CreateRandomInsideSpaceOfWidthNearWall(LabRoom* room, int& xOffset, int& yOffset, const int width, EDirectionEnum& direction, const bool canBeTaken = false);

	// Fills room with random objects, spawns and returns them
	// Should always be called on a room that is already spawned
	TArray<AActor*> FillRoom(LabRoom* room, int minNumOfLampsOverride = 0);

	// Activates all lamps in a single room
	void ActivateRoomLamps(LabRoom* room, bool forceAll = false);

	// Returns true if unexpanded rooms are reachable from here
	bool CanReachUnexpanded(LabRoom* start, TArray<LabRoom*>& checkedRooms);
	bool CanReachUnexpanded(LabRoom* start);

	// Expands room if it's not spawned yet
	// Repeats with all adjasent rooms recursively
	void ExpandInDepth(LabRoom* start, int depth, LabPassage* fromPassage, bool expandExpanded = false);
	void ExpandInDepth(LabRoom* start, int depth);
	// Spawns and fills room if it's not spawned yet
	// Repeats with all adjasent rooms recursively
	void SpawnFillInDepth(LabRoom* start, int depth, LabPassage* fromPassage, FVector initialPasLoc);
	void SpawnFillInDepth(LabRoom* start, int depth);

public:
	// Generates map
	UFUNCTION(BlueprintCallable, Category = "Map generation")
	void GenerateMap();
	// Resets the map
	UFUNCTION(BlueprintCallable, Category = "Map generation")
	void ResetMap();
	// Shows/hides debug
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ShowHideDebug();

protected:
	// For debug
	bool bShowDebug = false;

	// Rooms that are created but are not spawned yet and can still be changed
	TArray<LabRoom*> AllocatedRooms;

	// Room-specific space taken by various objects (not world locations but offsets)
	TMap<LabRoom*, TArray<FRectSpaceStruct>> AllocatedRoomSpace;

	// Rooms that have already been expanded
	TArray<LabRoom*> ExpandedRooms;

	// Rooms that were visited by player
	TArray<LabRoom*> VisitedRooms;
	// Number of visited overall (not same as VisitedRooms.Num() since rooms are removed from the array from time to time)
	int VisitedOverall;

	// Rooms that have their lamps turned on
	TArray<LabRoom*> RoomsWithLampsOn;

	// The room the character is in
	LabRoom* PlayerRoom; // Has an offset helping to avoid getting stuck in passage
	LabRoom* ActualPlayerRoom; 

	// Spawned map parts
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pools")
	TArray<TScriptInterface<IDeactivatable>> DoorcardPool;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pools")
	TArray<TScriptInterface<IDeactivatable>> ExitVolumePool;

	// Constants used for generation
	static const int ExpandDepth = 5;
	static const int SpawnFillDepth = 4;
	static const int ReshapeDarknessDepth = 3;
	static const int MaxFixDepth = 4;
	static const int MinRoomSize = 5;
	static const int MaxRoomSize = 35;
	static const int MinRoomArea = 25;
	static const int MaxRoomArea = 250;
	static const int MinRoomNumOfPassages = 1; // Can't be lower than 1
	static const int MaxRoomNumOfPassages = 8;
	static const int MaxRoomPassageCreationTriesPerDesired = 2; 
	static const int MinPassageWidth = 3; // Can't be lower than 2
	static const int MaxPassageWidth = 10; 
	static const int NormalDoorWidth = 4; // Can't be lower than 2
	static const int BigDoorWidth = 6;
	static const int ExitDoorWidth = 8;
	static const int MinVisitedBeforeExitCanSpawn = 25;
	static const int MinVisitedBeforeBlackDoorcardCanSpawn = 15;
	static const int MinDistanceBetweenPassages = 1; // Can't be lower than 1
	static const int MinDistanceInsideToPassage = 2; // Maybe it should be 1
	static const int MinRoomNumOfLamps = 0; 
	static const int MaxRoomNumOfLampsPerHundredArea = 2;
	static const int MaxRoomLampCreationTriesPerDesired = 2;
	static const int MinLampWidth = 1;
	static const int MaxLampWidth = 2;
	static const int MaxGenericSpawnTries = 3;
	static const int MinExpandTriesBeforeReshaping = 2;
	static const int MaxExpandTriesBeforeDisablingLights = 7;
	static const int MaxExpandTriesOverall = 10;
	// Probabilities
	static const float ReshapeDarknessOnEnterProbability;
	static const float ReshapeDarknessOnTickProbability;
	static const float LampsTurnOnOnEnterProbability;
	static const float LampsTurnOffPerSecondProbability;
	static const float AllLampsInRoomTurnOffProbability;
	static const float ConnectToOtherRoomProbability;
	static const float DeletePassageToFixProbability;
	static const float PassageIsDoorProbability;
	static const float DoorIsNormalProbability;
	static const float DoorIsExitProbability;
	static const float SpawnFlashlightProbability;
	static const float SpawnDoorcardProbability;
	static const float MakeRoomSpecialForCardProbability;
	static const float BlueProbability;
	static const float GreenProbability;
	static const float YellowProbability;
	static const float RedProbability;
	static const float BlackProbability;
	// Other constants
	static const float ReshapeDarknessTick;

	// Pointers to existing controllers and HUD
	UPROPERTY()
	class ADarknessController* DarknessController;
	UPROPERTY()
	class AMainPlayerController* MainPlayerController;
public:
	UPROPERTY()
	class UGameHUD* GameHUD;

	UPROPERTY()
	bool bHasWon = false;
	
private:
	// Classes used for spawning
	TSubclassOf<ABasicFloor> BasicFloorBP;
	TSubclassOf<ABasicWall> BasicWallBP;
	TSubclassOf<ABasicDoor> BasicDoorBP;
	TSubclassOf<AWallLamp> WallLampBP;
	TSubclassOf<AFlashlight> FlashlightBP;
	TSubclassOf<ALighter> LighterBP;
	TSubclassOf<ADoorcard> DoorcardBP;
	TSubclassOf<AExitVolume> ExitVolumeBP;

public:
	// Sets default values
	AMainGameMode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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