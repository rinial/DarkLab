// Fill out your copyright notice in the Description page of Project Settings.

#include "ExitVolume.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "MainCharacter.h"
#include "MainGameMode.h"

// Activates volume's light
void AExitVolume::ActivateLight()
{
	Light->SetVisibility(true);
}

void AExitVolume::Reset()
{
	Light->SetVisibility(false);
}

// Used for the collision overlaps
void AExitVolume::OnBeginOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{	
	AMainCharacter* character = Cast<AMainCharacter>(OtherActor);
	if (!character)
		return;

	Cast<AMainGameMode>(GetWorld()->GetAuthGameMode())->OnExitReached();
}
void AExitVolume::OnEndOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Left the exit! Why?? :c"));

	// TODO
}

// Sets default values
AExitVolume::AExitVolume()
{
	// Create root component
	Direction = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	SetRootComponent(Direction);

	// Create a box for collision
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	Collision->SetupAttachment(RootComponent);
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AExitVolume::OnBeginOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &AExitVolume::OnEndOverlap);

	// Create the light
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AExitVolume::BeginPlay()
{
	Super::BeginPlay();

	if (bForMenu)
		return;

	// Light is disabled
	Reset();

	// Set size
	// Gets overridden if done in constructor for some reason
	BaseSize = FIntVector(8, 1, 5); // TODO make bigger on X?
}