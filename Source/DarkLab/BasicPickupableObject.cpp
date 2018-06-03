// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicPickupableObject.h"
#include "MainCharacter.h"
#include "MainGameMode.h"
// TODO delete
#include "Doorcard.h"
#include "GameHUD.h"

// Called when the object is to be picked up
void ABasicPickupableObject::PickUp_Implementation(AMainCharacter * character)
{
	// We disable the object
	Execute_SetActive(this, false);

	Cast<AMainGameMode>(GetWorld()->GetAuthGameMode())->OnPickUp(this);
	

	// TODO delete
	ADoorcard* card = Cast<ADoorcard>(this);
	if (card)
	{
		FLinearColor color = card->GetColor();
		if (!character->HasDoorcardOfColor(color))
			character->GameHUD->ShowDoorcard(color);
	}


	character->Inventory.Add(this);

	UE_LOG(LogTemp, Warning, TEXT("Picked up %s"), *(Name.ToString()));
}
// Called when the object is activated
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