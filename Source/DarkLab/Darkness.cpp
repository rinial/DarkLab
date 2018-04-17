// Fill out your copyright notice in the Description page of Project Settings.

#include "Darkness.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "Runtime/Engine/Classes/Components/SphereComponent.h"
#include "Runtime/Engine/Classes/GameFramework/FloatingPawnMovement.h"

// Movement functions
void ADarkness::Move(const FVector direction)
{
	Movement->AddInputVector(direction);
}
void ADarkness::MoveToLocation(FVector location)
{
	TrackedLocation = location;
	TrackingType = ETrackingEnum::VE_Location;
}
void ADarkness::MoveToActor(AActor* actor)
{
	TrackedActor = actor;
	TrackingType = ETrackingEnum::VE_Actor;
}
void ADarkness::Stop()
{
	TrackingType = ETrackingEnum::VE_None;
}
// Track something
void ADarkness::Tracking()
{
	FVector currentLocation;
	switch (TrackingType)
	{
	case ETrackingEnum::VE_None:
		return;
	case ETrackingEnum::VE_Location:
		currentLocation = TrackedLocation;
		break;
	case ETrackingEnum::VE_Actor:
		if (!TrackedActor)
			return;
		currentLocation = TrackedActor->GetActorLocation();
		break;
	}

	FVector direction = currentLocation - GetActorLocation();

	// We don't move if objects are already close
	// TODO delete magic number
	if (direction.Size() < 1.0f)
		return;

	direction.Normalize();

	Move(direction);
}

// Sets default values
ADarkness::ADarkness()
{
	// Create a particle system
	DarkParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DarkParticles"));
	SetRootComponent(DarkParticles);

	// Create a sphere for collision
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	Collision->SetupAttachment(RootComponent);

	// Create a floating movement
	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));

 	// Set this pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADarkness::BeginPlay()
{
	Super::BeginPlay();

	// TODO
}

// Called every frame
void ADarkness::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO delete the "if" part
	if (bShouldTrack)
		Tracking();
}