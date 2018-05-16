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
	// TODO delete, this is only used for debug to prevent darkness from moving
	if (bIsStationary)
		return;

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
	TrackedActor = actor;
	TrackingType = ETrackingEnum::VE_Actor;
}
void ADarkness::Stop()
{
	TrackingType = ETrackingEnum::VE_None;
}
// When light is too strong goes backwards and returns true. Great for some situations and will look weird in others. Returns false if light aint too strong
bool ADarkness::RetreatFromLight()
{
	// Retreats faster in brighter light, but resistance helps
	float temp = 1 - LightFearK * Luminosity * FMath::Max(0.0f, Luminosity - LightResistance);
	if (temp >= 0)
		return false; // Doesn't retreat
	
	// Away from brightest light
	FVector fleeDirection = GetActorLocation() - BrightestLightLocation;
	fleeDirection.Normalize();
	Movement->AddInputVector(fleeDirection * temp * -1); // temo is lower than 0 at this point
	
	return true; // Does retreat
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
	if (direction.Size() < 20.0f)
		return;

	direction.Normalize();

	Move(direction);
}
// Goes away from last brightest light
void ADarkness::IntoDarkness()
{
	// Away from brightest light
	FVector fleeDirection = GetActorLocation() - BrightestLightLocation;
	fleeDirection.Normalize();
	Move(fleeDirection);
}

// Used for collision overlaps
void ADarkness::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// UE_LOG(LogTemp, Warning, TEXT("Entered the darkness!"));
	
	// Darkness is harmless when it isn't hunting
	if (DarknessController->State != EDarkStateEnum::VE_Hunting)
		return;

	AMainCharacter* character = Cast<AMainCharacter>(OtherActor);
	if (!character)
		return;

	character->Disable();

	// Tell the controller that darkness disabled player
	DarknessController->OnDisabling();
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
	DarknessController = Cast<ADarknessController>(GetController());
}

// Called every frame
void ADarkness::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// We check the light level	
	Luminosity = GameMode->GetLightingAmount(BrightestLightLocation, this, true, Collision->GetScaledSphereRadius() + 10, true, bShowLightDebug);

	// Increase resistance if stuck in bright light
	if (Luminosity > LightResistance)
		LightResistance += DeltaTime * LightResGainSpeed;
	// Or decrease it if in no light at all
	else if (Luminosity <= 0.0f && LightResistance > 0.0f)
		LightResistance -= DeltaTime * LightResLossSpeed;
	if (LightResistance < 0.0f)
		LightResistance = 0.0f;

	// TODO delete later: used for debug
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Light resistance: %f"), LightResistance), true);

		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Darkness luminosity: %f"), Luminosity), true);
	}
}