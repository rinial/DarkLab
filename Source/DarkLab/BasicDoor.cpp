// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicDoor.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "MainCharacter.h"

// Called when the object is activated
void ABasicDoor::Activate_Implementation(AMainCharacter * character)
{
	if (DoorDriver->GetPlaybackPosition() == 0.0f
		|| DoorDriver->IsReversing())
	{
		UE_LOG(LogTemp, Warning, TEXT("Opened door"));
		DoorDriver->Play();
	}
	else if (DoorDriver->GetPlaybackPosition() == DoorDriver->GetTimelineLength()
		|| DoorDriver->IsPlaying())
	{
		UE_LOG(LogTemp, Warning, TEXT("Closed door"));
		DoorDriver->Reverse();
	}
}

// Sets default values
ABasicDoor::ABasicDoor()
{
	// Create root component
	Direction = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	SetRootComponent(Direction);

	// Create the door's frame
	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	DoorFrame->SetupAttachment(RootComponent);

	// Create the door's driver
	DoorDriver = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorDriver"));

	// Fill information
	Name = NSLOCTEXT("LocalNS", "Basic door name", "Door");
	BasicInfo = NSLOCTEXT("LocalNS", "Basic door information", "Can be opened and closed");
}

// Called when the game starts or when spawned
void ABasicDoor::BeginPlay()
{
	Super::BeginPlay();

	// Set size
	// Gets overridden if done in constructor for some reason
	BaseSize = FIntVector(8, 1, 5);
}