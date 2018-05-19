// Fill out your copyright notice in the Description page of Project Settings.

#include "Flashlight.h"
#include "Components/ArrowComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"

// Enables or disables light
void AFlashlight::Use_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Toggled %s"), *(Name.ToString()));

	MainLight->ToggleVisibility();
	ExtraLight->ToggleVisibility();
}

// Resets to initial state
void AFlashlight::Reset()
{
	MainLight->SetVisibility(false);
	ExtraLight->SetVisibility(false);
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

	// Fill information
	Name = NSLOCTEXT("LocalNS", "Flashlight name", "Flashlight");
	BasicInfo = NSLOCTEXT("LocalNS", "Flashlight information", "Provides light in a cone");

	ZOffset = 5.0f;
}