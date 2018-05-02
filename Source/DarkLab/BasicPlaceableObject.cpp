// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicPlaceableObject.h"

// Represents objects with physical represention that can be placed on the map
FIntVector ABasicPlaceableObject::GetSize_Implementation()
{
	FVector scale = GetActorScale();
	return FIntVector(Size.X * scale.X, Size.Y * scale.Y, Size.Z * scale.Z);
}
// Places the object on the map, using bottom left corner
void ABasicPlaceableObject::Place_Implementation(FIntVector botLeftLoc)
{
	FIntVector size = Execute_GetSize(this);
	SetActorLocation(FVector(botLeftLoc * 50) + FVector(size.X * 25, size.Y * 25, 0)); // - FVector(25, 25, 0));
}