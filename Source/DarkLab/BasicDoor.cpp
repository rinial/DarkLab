// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicDoor.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "MainCharacter.h"

// Called when the object is activated
void ABasicDoor::ActivateObject(AMainCharacter * character)
{
	if (DoorColor != FLinearColor::White && !character->HasDoorcardOfColor(DoorColor))
	{
		UE_LOG(LogTemp, Warning, TEXT("You can't open this door"));
		return;
	}

	if (DoorDriver->GetPlaybackPosition() == 0.0f
		|| DoorDriver->IsReversing())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Opened %s"), *(Name.ToString()));
		DoorDriver->Play();
	}
	else if (DoorDriver->GetPlaybackPosition() == DoorDriver->GetTimelineLength()
		|| DoorDriver->IsPlaying())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Closed %s"), *(Name.ToString()));
		DoorDriver->Reverse();
	}
}

// TODO let some interface define it?
// Resets to initial state
void ABasicDoor::Reset()
{
	DoorDriver->Stop();
	DoorDriver->SetPlaybackPosition(0.0f, true);
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

	// Set activatable parameters
	bActivatableDirectly = true;
	bActivatableIndirectly = true;
}

// Called when the game starts or when spawned
void ABasicDoor::BeginPlay()
{
	Super::BeginPlay();

	// Set size
	// Gets overridden if done in constructor for some reason
	BaseSize = FIntVector(4, 1, 5);
}