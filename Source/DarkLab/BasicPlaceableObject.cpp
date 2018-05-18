// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicPlaceableObject.h"
#include "MainGameMode.h"

// Returns the size of the object in cells
FIntVector ABasicPlaceableObject::GetSize_Implementation()
{
	FVector scale = GetActorScale();
	if(GridDirection == EDirectionEnum::VE_Up || GridDirection == EDirectionEnum::VE_Down)
		return FIntVector(BaseSize.X * scale.Y, BaseSize.Y * scale.X, BaseSize.Z * scale.Z);
	return FIntVector(BaseSize.Y * scale.X, BaseSize.X * scale.Y, BaseSize.Z * scale.Z);
}
// Tries to set new size of the object in cells, returns success
// Actually sets not BaseSize but actor's scale, which affects GetSize
// Assumes direction to be Up. Note: should do Place after this
bool ABasicPlaceableObject::SetSize_Implementation(const FIntVector size)
{
	if (size.X < 1 || size.Y < 1 || size.Z < 1)
		return false;

	// if (size.X < BaseSize.X || size.Y < BaseSize.Y || size.Z < BaseSize.Z)
	// 	return false;

	// if (size.X % BaseSize.X != 0 || size.Y % BaseSize.Y != 0 || size.Z % BaseSize.Z != 0)
	//	return false;

	SetActorScale3D(FVector(size.Y * 1.0f / BaseSize.Y, size.X * 1.0f / BaseSize.X, size.Z * 1.0f / BaseSize.Z));
	return true;
}
bool ABasicPlaceableObject::SetSizeXY_Implementation(const int x, const int y)
{
	if (x < 1 || y < 1)
		return false;

	// if (x < BaseSize.X || y < BaseSize.Y)
	// 	return false;

	// if (x % BaseSize.X != 0 || y % BaseSize.Y != 0)
	// 	return false;

	SetActorScale3D(FVector(y * 1.0f / BaseSize.Y, x * 1.0f / BaseSize.X, GetActorScale3D().Z));
	return true;
}
// Places the object on the map, using bottom left corner
void ABasicPlaceableObject::Place_Implementation(const FIntVector botLeftLoc, const EDirectionEnum direction)
{
	GridDirection = direction;
	FIntVector size = Execute_GetSize(this);

	FVector location;
	AMainGameMode::GridToWorld(botLeftLoc.X, botLeftLoc.Y, size.X, size.Y, location.X, location.Y);
	location.Z = ZOffset;
	SetActorLocation(location); //, false, nullptr, ETeleportType::TeleportPhysics);

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