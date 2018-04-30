// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicEquipableObject.h"
#include "MainCharacter.h"

// Called when the object is to be equiped
void ABasicEquipableObject::Equip_Implementation(AMainCharacter* character, FName location)
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

// Called when the object is used (picked up in the laboratory)
void ABasicEquipableObject::Activate_Implementation(AMainCharacter* character)
{
	// It's not activatable animore
	character->ActivatableObjects.Remove(this);

	// TODO this should put item into inventory, not instantly equip it
	Execute_Equip(this, character, FName("LeftHand"));
}

// Returns object's name
FText ABasicEquipableObject::GetName_Implementation()
{
	return Name;
}
// Returns basic infomation about the object
FText ABasicEquipableObject::GetBasicInfo_Implementation()
{
	return BasicInfo;
}

// Sets default values
ABasicEquipableObject::ABasicEquipableObject()
{
 	// Set this actor to NOT call Tick() every frame
	PrimaryActorTick.bCanEverTick = false;
	// TODO enable ticks?
}

// Called when the game starts or when spawned
void ABasicEquipableObject::BeginPlay()
{
	Super::BeginPlay();
	
	// TODO
}

// Called every frame
void ABasicEquipableObject::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO enable ticks?
}