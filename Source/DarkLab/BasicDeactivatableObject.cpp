// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicDeactivatableObject.h"

// Activates/deactivates the object (usually for pooling)
void ABasicDeactivatableObject::SetActive_Implementation(bool active)
{
	if (bIsActive == active)
		return;

	bIsActive = active;

	SetActorHiddenInGame(!active);
	SetActorEnableCollision(active);
	if (bDefaultTickEnabled)
		SetActorTickEnabled(active);

	// TODO add specific disables for different possible components
	/*UStaticMeshComponent* meshComp = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
	if (meshComp)
	{
		if (active)
			meshComp->Activate();
		else
			meshComp->Deactivate();
	}*/
}
// Returns true if the object is active
bool ABasicDeactivatableObject::IsActive_Implementation()
{
	return bIsActive;
}

// Sets default values
ABasicDeactivatableObject::ABasicDeactivatableObject()
{
	PrimaryActorTick.bCanEverTick = bDefaultTickEnabled;
}