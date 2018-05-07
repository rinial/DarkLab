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
class AFlashlight;

// Controls the game
UCLASS(Blueprintable)
class DARKLAB_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Returns the light level and the location of the brightest light
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const bool sixPoints = false, const float sixPointsRadius = 30.0f);
	float GetLightingAmount(FVector& lightLoc, const FVector location, const bool sixPoints = false, const float sixPointsRadius = 30.0f);
	float GetLightingAmount(FVector& lightLoc, const TArray<FVector> locations);
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const FVector location, const bool sixPoints = false, const float sixPointsRadius = 30.0f);
	float GetLightingAmount(FVector& lightLoc, const AActor* actor, const TArray<FVector> locations);

protected:
	// Places an object on the map (size < 1 means that we take current size for this axis)
	// TODO return false if can't place?
	void PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftLocX, const int botLeftLocY, const EDirectionEnum direction = EDirectionEnum::VE_Up, const bool setSizeFirst = false, const int sizeX = 1, const int sizeY = 1, const int sizeZ = 0);
	void PlaceObject(TScriptInterface<IPlaceable> object, const int botLeftLocX, const int botLeftLocY, const int botLeftLocZ, const EDirectionEnum direction = EDirectionEnum::VE_Up, const bool setSizeFirst = false, const int sizeX = 1, const int sizeY = 1, const int sizeZ = 0);
	void PlaceObject(TScriptInterface<IPlaceable> object, const FIntVector botLeftLoc = FIntVector(0, 0, 0), const EDirectionEnum direction = EDirectionEnum::VE_Up, const bool setSizeFirst = false, const int sizeX = 1, const int sizeY = 1, const int sizeZ = 0);

	// Gets the pool for the object/class
	TArray<TScriptInterface<IDeactivatable>>& GetCorrectPool(TScriptInterface<IDeactivatable> object);
	TArray<TScriptInterface<IDeactivatable>>& GetCorrectPool(UClass* cl);

	// Deactivates and adds to a pool
	UFUNCTION(BlueprintCallable, Category = "Pools")
	void PoolObject(TScriptInterface<IDeactivatable> object);
	UFUNCTION(BlueprintCallable, Category = "Pools")
	void PoolObjects(TArray<TScriptInterface<IDeactivatable>>& objects);

	// Tries to find a poolable object in a specified array
	UFUNCTION(BlueprintCallable, Category = "Pools")
	UObject* TryGetPoolable(UClass* cl);

	// Spawn specific objects
	UFUNCTION(BlueprintCallable, Category = "Spawns")
	ABasicFloor* SpawnBasicFloor(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY);
	UFUNCTION(BlueprintCallable, Category = "Spawns")
	ABasicWall* SpawnBasicWall(const int botLeftX, const int botLeftY, const int sizeX, const int sizeY);
	UFUNCTION(BlueprintCallable, Category = "Spawns")
	ABasicDoor* SpawnBasicDoor(const int botLeftX, const int botLeftY, const EDirectionEnum direction, const FLinearColor color = FLinearColor::White, const int width = 4);
	UFUNCTION(BlueprintCallable, Category = "Spawns")
	AFlashlight* SpawnFlashlight(const int botLeftX, const int botLeftY, const EDirectionEnum direction = EDirectionEnum::VE_Up);

	// Spawn full parts of the lab
	TArray<TScriptInterface<IDeactivatable>> SpawnRoom(class LabRoom* room);
	
private:
	// Classes used for spawning
	TSubclassOf<ABasicFloor> BasicFloorBP;
	TSubclassOf<ABasicWall> BasicWallBP;
	TSubclassOf<ABasicDoor> BasicDoorBP;
	TSubclassOf<AFlashlight> FlashlightBP;

protected:
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
	TArray<TScriptInterface<IDeactivatable>> FlashlightPool;

public:
	// Sets default values
	AMainGameMode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};