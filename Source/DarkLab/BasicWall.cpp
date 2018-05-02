// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicWall.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ABasicWall::ABasicWall()
{
	// Create root component
	Direction = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	SetRootComponent(Direction);

	// Create the wall's shape
	Wall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	Wall->SetupAttachment(RootComponent);
}