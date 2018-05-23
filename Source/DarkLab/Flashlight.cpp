// Fill out your copyright notice in the Description page of Project Settings.

#include "Flashlight.h"
#include "Components/ArrowComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"

// Enables or disables light
void AFlashlight::Use_Implementation()
{
	if (PowerLevel > 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Toggled %s"), *(Name.ToString()));

		MainLight->ToggleVisibility();
		ExtraLight->ToggleVisibility();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT(" %s is out of power"), *(Name.ToString()));
	}
}

// Resets to initial state
void AFlashlight::Reset()
{
	MainLight->SetVisibility(false);
	ExtraLight->SetVisibility(false);
	ResetPowerLevel();
}

// Resets only power level
void AFlashlight::ResetPowerLevel()
{
	PowerLevel = 1.f;
	MainLight->SetLightColor(NormalColor * PowerLevel);
	ExtraLight->SetLightColor(NormalColor * PowerLevel);
}

// Returns true if flashlight is on
bool AFlashlight::IsOn()
{
	return MainLight->IsVisible();
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
	BasicInfo = NSLOCTEXT("LocalNS", "Flashlight information", "Provides light in a cone. Loses power in time");

	ZOffset = 5.0f;

	bDefaultTickEnabled = true;
}

// Called every frame
void AFlashlight::Tick(const float deltaTime)
{
	Super::Tick(deltaTime);

	if (MainLight->IsVisible())
	{
		PowerLevel -= PowerLossPerSecond * deltaTime;
		PowerLevel = PowerLevel > 0.f ? PowerLevel : 0.f;
		MainLight->SetLightColor(NormalColor * PowerLevel);
		ExtraLight->SetLightColor(NormalColor * PowerLevel);

		if (PowerLevel == 0.f)
		{
			UE_LOG(LogTemp, Warning, TEXT(" %s lost all power"), *(Name.ToString()));
			MainLight->ToggleVisibility();
			ExtraLight->ToggleVisibility();
		}
	}
}