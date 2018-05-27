// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicDoor.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "MainCharacter.h"
#include "MainGameMode.h"
#include "GameHUD.h"

// Called when the object is activated
void ABasicDoor::ActivateObject(AMainCharacter * character)
{
	if (DoorColor != FLinearColor::White && !character->HasDoorcardOfColor(DoorColor))
	{
		if (!bIsExit)
			character->GameHUD->ShowHideWarning(true, FText::FromString("You don't have a keycard for this door"));
		else
			character->GameHUD->ShowHideWarning(true, FText::FromString("You reached the exit, but you don't have a suitable keycard"));
		UE_LOG(LogTemp, Warning, TEXT("You can't open this door"));
		return;
	}

	if (DoorDriver->GetPlaybackPosition() == 0.0f
		|| DoorDriver->IsReversing())
	{
		if (bIsExit)
			Cast<AMainGameMode>(GetWorld()->GetAuthGameMode())->OnExitOpened(this);

		// UE_LOG(LogTemp, Warning, TEXT("Opened %s"), *(Name.ToString()));
		DoorDriver->Play();
	}
	else if (!bIsExit && (DoorDriver->GetPlaybackPosition() == DoorDriver->GetTimelineLength() || DoorDriver->IsPlaying())) // We can't close exit door
	{
		// UE_LOG(LogTemp, Warning, TEXT("Closed %s"), *(Name.ToString()));
		DoorDriver->Reverse();
	}
}

// TODO let some interface define it?
// Resets to initial state
void ABasicDoor::ResetDoor(bool isExit)
{
	DoorDriver->Stop();
	DoorDriver->SetPlaybackPosition(0.0f, true);
	DoorDriver->SetPlayRate(!isExit ? 1.f : 0.3f);
	if (!isExit)
	{
		DoorDriver->SetPlayRate(1.f);
		// Fill information
		Name = NSLOCTEXT("LocalNS", "Basic door name", "Door");
		BasicInfo = NSLOCTEXT("LocalNS", "Basic door information", "Can be opened or closed with a keycard of similar color");
	}
	else
	{
		DoorDriver->SetPlayRate(0.3f);
		// Fill information
		Name = NSLOCTEXT("LocalNS", "Exit door name", "Exit");
		BasicInfo = NSLOCTEXT("LocalNS", "Exit door information", "Leads out of the lab. Can be opened with a black keycard");
	}
	bIsExit = isExit;
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
	BasicInfo = NSLOCTEXT("LocalNS", "Basic door information", "Can be opened or closed with a keycard of similar color");

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