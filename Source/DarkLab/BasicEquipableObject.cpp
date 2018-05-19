// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicEquipableObject.h"
#include "MainCharacter.h"

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

	// Then equip
	Execute_Equip(this, character, FName("LeftHand"));
}

//// Sets default values
//ABasicEquipableObject::ABasicEquipableObject()
//{
//	// Set activatable parameters
//	bActivatableDirectly = true;
//	bActivatableIndirectly = false;
//}