// Fill out your copyright notice in the Description page of Project Settings.

#include "Flashlight.h"
#include "Components/ArrowComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"

// Enables or disables light
void AFlashlight::Use_Implementation()
{
	bLight = !bLight;
	MainLight->bIsActive = bLight;
	ExtraLight->bIsActive = bLight;
}

// Sets default values
AFlashlight::AFlashlight()
{
	// Create root component
	Direction = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	SetRootComponent(Direction);

	// Create the main light
	MainLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("MainLight"));
	MainLight->SetupAttachment(RootComponent);

	// Create the extra light
	ExtraLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ExtraLight"));
	ExtraLight->SetupAttachment(RootComponent);
}