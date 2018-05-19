// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicPickupableObject.h"
#include "MainCharacter.h"
#include "MainGameMode.h"

void ABasicPickupableObject::PickUp_Implementation(AMainCharacter * character)
{
	// We disable the object
	Execute_SetActive(this, false);
	
	character->Inventory.Add(this);
	Cast<AMainGameMode>(GetWorld()->GetAuthGameMode())->OnPickUp(this);

	UE_LOG(LogTemp, Warning, TEXT("Picked up %s"), *(Name.ToString()));
}

void ABasicPickupableObject::ActivateObject(AMainCharacter * character)
{
	if (!character)
		return;

	Execute_PickUp(this, character);

	// It's not activatable animore
	character->ActivatableObjects.Remove(this);
}

// Sets default values
ABasicPickupableObject::ABasicPickupableObject()
{
	// Set activatable parameters
	bActivatableDirectly = true;
	bActivatableIndirectly = false;
}