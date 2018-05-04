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

	UE_LOG(LogTemp, Warning, TEXT("Equiped"));
}
// Called when the object is to be unequiped
void ABasicEquipableObject::Unequip_Implementation(AMainCharacter* character)
{
	character->EquipedObject = nullptr;

	// TODO just move to inventory
	this->Destroy();

	UE_LOG(LogTemp, Warning, TEXT("Unequiped"));
}

// Called when the object is activated (picked up in the laboratory)
void ABasicEquipableObject::Activate_Implementation(AMainCharacter* character)
{
	// TODO add object name
	// UE_LOG(LogTemp, Warning, FString::Printf(TEXT("Picked up %s"), *Name.ToString()));
	UE_LOG(LogTemp, Warning, TEXT("Picked up"));

	// It's not activatable animore
	character->ActivatableObjects.Remove(this);

	// TODO this should put item into inventory, not instantly equip it
	Execute_Equip(this, character, FName("LeftHand"));
}