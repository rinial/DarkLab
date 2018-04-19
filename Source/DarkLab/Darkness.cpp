// Fill out your copyright notice in the Description page of Project Settings.

#include "Darkness.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "DarknessController.h"
#include "MainCharacter.h"
#include "MainGameMode.h"
// For on screen debug
#include "EngineGlobals.h"
#include "Engine/Engine.h"

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
	// We check the light level
	float luminosity = GameMode->GetLightingAmount(this, true, 60);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Darkness luminosity: %f"), luminosity), true);

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
	if (direction.Size() < 10.0f)
		return;

	direction.Normalize();

	Move(direction);
}

// Used for collision overlaps
void ADarkness::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// UE_LOG(LogTemp, Warning, TEXT("Entered the darkness!"));
	
	AMainCharacter* character = Cast<AMainCharacter>(OtherActor);
	if (!character)
		return;

	character->Disable();

	// Tell the controller that darkness disabled player
	ADarknessController* controller = Cast<ADarknessController>(GetController());
	if (controller)
		controller->OnDisabling();
}
void ADarkness::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	// UE_LOG(LogTemp, Warning, TEXT("Escaped the darkness!"));

	// TODO
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
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ADarkness::OnBeginOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &ADarkness::OnEndOverlap);

	// Create a floating movement
	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));

 	// Set this pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADarkness::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
}

// Called every frame
void ADarkness::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO delete the "if" part
	if (bShouldTrack)
		Tracking();
}