// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicPlaceableObject.h"

// Represents objects with physical represention that can be placed on the map
FIntVector ABasicPlaceableObject::GetSize_Implementation()
{
	FVector scale = GetActorScale();
	if(GridDirection == EDirectionEnum::VE_Up || GridDirection == EDirectionEnum::VE_Down)
		return FIntVector(Size.X * scale.Y, Size.Y * scale.X, Size.Z * scale.Z);
	return FIntVector(Size.Y * scale.X, Size.X * scale.Y, Size.Z * scale.Z);
}
// Places the object on the map, using bottom left corner
void ABasicPlaceableObject::Place_Implementation(FIntVector botLeftLoc, EDirectionEnum direction)
{
	GridDirection = direction;
	FIntVector size = Execute_GetSize(this);

	SetActorLocation(FVector(botLeftLoc * 50) + FVector(size.Y * 25, size.X * 25, 0)); // - FVector(25, 25, 0));

	float rotation;
	switch (GridDirection)
	{
	case EDirectionEnum::VE_Up:
		rotation = 0;
		break;
	case EDirectionEnum::VE_Right:
		rotation = 90;
		break;
	case EDirectionEnum::VE_Down:
		rotation = 180;
		break;
	case EDirectionEnum::VE_Left:
		rotation = -90;
		break;
	}

	SetActorRotation(FQuat(FVector(0,0,1), FMath::DegreesToRadians(rotation)));
}