// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicPlaceableObject.h"
#include "BasicWall.generated.h"

// A simple wall
UCLASS()
class DARKLAB_API ABasicWall : public ABasicPlaceableObject
{
	GENERATED_BODY()
	
protected:
	// Wall's direction and root object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall: Components")
	class UArrowComponent* Direction;

	// Wall's shape
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall: Components")
	class UStaticMeshComponent* Wall;

public:
	// Sets default values
	ABasicWall();
};