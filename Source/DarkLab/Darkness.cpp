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
	// Moves slower in light, but light resistance helps
	float temp = 1 - LightFearK * Luminosity * FMath::Max(0.0f, Luminosity - LightResistance);
	Movement->AddInputVector(direction * FMath::Max(0.0f, temp));
}
void ADarkness::MoveToLocation(FVector location)
{
	TrackedLocation = location;
	TrackingType = ETrackingEnum::VE_Location;
}
void ADarkness::MoveToActor(AActor* actor)
{
	// TODO move somewhere
	// Starts hunting
	State = EDarkStateEnum::VE_Hunting;

	TrackedActor = actor;
	TrackingType = ETrackingEnum::VE_Actor;
}
void ADarkness::Stop()
{
	State = EDarkStateEnum::VE_Passive;
	TrackingType = ETrackingEnum::VE_None;
}
// Like Move but it sometimes goes backwards when light is too strong, which is great for some situations and will look weird in others
void ADarkness::MoveWithFear(const FVector direction)
{
	// Moves slower in light, but light resistance helps
	// In high luminosity the darkness can actually retreat
	float temp = 1 - LightFearK * Luminosity * FMath::Max(0.0f, Luminosity - LightResistance);
	if(temp >= 0)
		Movement->AddInputVector(direction * temp); // same as normal Move
	else
	{
		FVector fleeDirection = GetActorLocation() - BrightestLightLocation;
		fleeDirection.Normalize();
		Movement->AddInputVector(fleeDirection * temp * -1);
	}
	/*Movement->AddInputVector(direction * (1 - LightFearK * Luminosity * FMath::Max(0.0f, Luminosity - LightResistance)));*/
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
	if (direction.Size() < 10.0f)
		return;

	direction.Normalize();

	// TODO maybe normal Move shoudld be used
	// Move(direction);
	MoveWithFear(direction);
}

// Used for collision overlaps
void ADarkness::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// UE_LOG(LogTemp, Warning, TEXT("Entered the darkness!"));
	
	// Darkness is harmless when it isn't hunting
	if (State != EDarkStateEnum::VE_Hunting)
		return;

	AMainCharacter* character = Cast<AMainCharacter>(OtherActor);
	if (!character)
		return;

	Stop();

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

	// TODO delete from here later. we should evaluate it less often
	// We check the light level	
	Luminosity = GameMode->GetLightingAmount(BrightestLightLocation, this, true, Collision->GetScaledSphereRadius());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Light resistance: %f"), LightResistance), true);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Darkness luminosity: %f"), Luminosity), true);
	}

	// Increase resistance if stuck in light
	if (Luminosity > LightResistance)
		LightResistance += DeltaTime * LightResSpeed;

	// TODO delete the "if" part
	if (bShouldTrack)
		Tracking();
}