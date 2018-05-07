// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicActivatableObject.h"

// Called when the object is activated by character
void ABasicActivatableObject::Activate_Implementation(AMainCharacter * character)
{
	if (!bActivatableDirectly)
		return;

	ActivateObject(character);
}
// Called when the object is activated indirectly
void ABasicActivatableObject::ActivateIndirectly_Implementation()
{
	if (!bActivatableIndirectly)
		return;

	ActivateObject(nullptr);
}
// Returns true if activatable directly
bool ABasicActivatableObject::IsActivatableDirectly_Implementation()
{
	return bActivatableDirectly;
}
// Returns true if activatable indirectly
bool ABasicActivatableObject::IsActivatableIndirectly_Implementation()
{
	return bActivatableIndirectly;
}

// Called when the object is activated (should always be overridden)
void ABasicActivatableObject::ActivateObject(AMainCharacter * character)
{
	UE_LOG(LogTemp, Warning, TEXT("Activated"));
}