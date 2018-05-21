// Fill out your copyright notice in the Description page of Project Settings.

#include "Lighter.h"
#include "Components/ArrowComponent.h"
#include "Components/PointLightComponent.h"

// Enables or disables light
void ALighter::Use_Implementation()
{
	if (PowerLevel > 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Toggled %s"), *(Name.ToString()));

		Light->ToggleVisibility();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT(" %s is out of gas"), *(Name.ToString()));
	}
}

// Resets to initial state
void ALighter::Reset()
{
	Light->SetVisibility(false);
	ResetPowerLevel();
}

// Resets only power level
void ALighter::ResetPowerLevel()
{
	PowerLevel = 1.f;
	CurrentlySeenLevel = 0.75f;
	Light->SetLightColor(NormalColor * CurrentlySeenLevel);
}

// Sets default values
ALighter::ALighter()
{
	// Create root component
	Direction = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	SetRootComponent(Direction);

	// Create the light
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(RootComponent);

	// Fill information
	Name = NSLOCTEXT("LocalNS", "Lighter name", "Lighter");
	BasicInfo = NSLOCTEXT("LocalNS", "Lighter information", "Provides light around");

	ZOffset = 5.0f;

	bDefaultTickEnabled = true;

	NormalColor = FLinearColor(1.f, 0.5f, 0.06f);
}

// Called every frame
void ALighter::Tick(const float deltaTime)
{
	Super::Tick(deltaTime);

	if (Light->IsVisible())
	{
		PowerLevel -= PowerLossPerSecond * deltaTime;
		PowerLevel = PowerLevel > 0.f ? PowerLevel : 0.f;
		if (PowerLevel != 0.f)
		{
			float bottomLine = PowerLevel / 2.f;
			float topLine = PowerLevel;

			float min = (bottomLine + CurrentlySeenLevel) / 2.f;
			float max = (topLine + CurrentlySeenLevel) / 2.f;

			CurrentlySeenLevel = FMath::FRandRange(min, max);
		}
		else
			CurrentlySeenLevel = 0.f;
		Light->SetLightColor(NormalColor * CurrentlySeenLevel);

		if (PowerLevel == 0.f)
		{
			UE_LOG(LogTemp, Warning, TEXT(" %s lost all gas"), *(Name.ToString()));
			Light->ToggleVisibility();
		}
	}
}