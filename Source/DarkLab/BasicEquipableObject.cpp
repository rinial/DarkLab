// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicEquipableObject.h"
#include "MainCharacter.h"
#include "Components/ArrowComponent.h"

// Called when the object is to be equiped
void ABasicEquipableObject::Equip_Implementation(AMainCharacter* character)
{
	// TODO move it to function parameters
	FName location = FName("LeftHand");

	// TOTO change later
	USceneComponent* mesh = Cast<USceneComponent>(character->GetMesh());
	if (!mesh)
		return;

	AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, location);
	character->EquipedObject = this;
}
// Called when the object is to be unequiped
void ABasicEquipableObject::Unequip_Implementation(AMainCharacter* character)
{
	// TODO
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