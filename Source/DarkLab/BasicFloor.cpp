// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicFloor.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ABasicFloor::ABasicFloor()
{
	// Create root component
	Direction = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	SetRootComponent(Direction);

	// Create the floor's shape
	Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	Floor->SetupAttachment(RootComponent);
}