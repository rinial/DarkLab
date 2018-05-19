// Fill out your copyright notice in the Description page of Project Settings.

#include "Doorcard.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets the color
void ADoorcard::SetColor(FLinearColor color)
{
	Color = color;
	UpdateMeshColor(color);
}
// Returns the color
FLinearColor ADoorcard::GetColor()
{
	return Color;
}

// Update's the color of the doorcard mesh
void ADoorcard::UpdateMeshColor(FLinearColor color)
{
	Card->SetVectorParameterValueOnMaterials("Color", FVector(color.R, color.G, color.B));
}

// Sets default values
ADoorcard::ADoorcard()
{
	// Create root component
	Direction = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	SetRootComponent(Direction);

	// Create the card itself
	Card = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Card"));
	Card->SetupAttachment(RootComponent);
	
	// Fill information
	Name = NSLOCTEXT("LocalNS", "Doorcard name", "Doorcard");
	BasicInfo = NSLOCTEXT("LocalNS", "Doorcard information", "Use to open a colored door");

	ZOffset = 1.0f;
}