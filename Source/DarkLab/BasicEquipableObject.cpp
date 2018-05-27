// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicEquipableObject.h"
#include "MainCharacter.h"
// TODO delete later
#include "Flashlight.h"
#include "Lighter.h"
#include "GameHUD.h"

// Called when the object is to be equiped
void ABasicEquipableObject::Equip_Implementation(AMainCharacter* character, const FName location)
{
	// We find the mesh and attach object to it
	USceneComponent* mesh = Cast<USceneComponent>(character->GetMesh());
	if (!mesh) return;
	AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, location);

	// We unequip previously equiped object
	TScriptInterface<IEquipable> currentlyEquiped = character->EquipedObject;
	if (currentlyEquiped)
		currentlyEquiped->Execute_Unequip(currentlyEquiped->_getUObject(), character);

	character->EquipedObject = this;
	character->SetOutline(this, false);

	// Finally we activate it to be seen 
	Execute_SetActive(this, true);

	UE_LOG(LogTemp, Warning, TEXT("Equiped %s"), *(Name.ToString()));
}
// Called when the object is to be unequiped
void ABasicEquipableObject::Unequip_Implementation(AMainCharacter* character)
{
	// We disable the object
	Execute_SetActive(this, false);

	// No need to destroy it
	character->EquipedObject = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("Unequiped %s"), *(Name.ToString()));
}

// Called when the object is activated (picked up in the laboratory)
void ABasicEquipableObject::ActivateObject(AMainCharacter* character)
{
	if (!character)
		return;

	// Pick up first
	Super::ActivateObject(character);

	
	// TODO delete later
	AFlashlight* flashlight = Cast<AFlashlight>(this);
	if (flashlight)
	{
		if (character->FlashlightIndex < 0)
		{
			character->FlashlightIndex = character->Inventory.Num() - 1;
			character->GameHUD->ShowItem2Panel();
			
			if (!character->EquipedObject)
				Execute_Equip(this, character, FName("LeftHand"));
		}
		else
			Cast<AFlashlight>(character->Inventory[character->FlashlightIndex]->_getUObject())->ResetPowerLevel();
		return;
	}
	ALighter* lighter = Cast<ALighter>(this);
	if (lighter)
	{
		if (character->LighterIndex < 0)
		{
			character->LighterIndex = character->Inventory.Num() - 1;

			if (!character->EquipedObject)
				Execute_Equip(this, character, FName("LeftHand"));
		}
		else
			Cast<ALighter>(character->Inventory[character->LighterIndex]->_getUObject())->ResetPowerLevel();
		return;
	}
	

	// We equip directly after picking up only if there is nother else equiped
	// if (!character->EquipedObject)
	//	Execute_Equip(this, character, FName("LeftHand"));
}

//// Sets default values
//ABasicEquipableObject::ABasicEquipableObject()
//{
//	// Set activatable parameters
//	bActivatableDirectly = true;
//	bActivatableIndirectly = false;
//}