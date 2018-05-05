// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicPlaceableObject.h"
#include "BasicFloor.generated.h"

// A simple floor
UCLASS()
class DARKLAB_API ABasicFloor : public ABasicPlaceableObject
{
	GENERATED_BODY()
	
protected:
	// Floor's direction and root object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Floor: Components")
	class UArrowComponent* Direction;

	// Floor's shape
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Floor: Components")
	class UStaticMeshComponent* Floor;

public:
	// Sets default values
	ABasicFloor();
};