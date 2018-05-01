// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicInformativeObject.h"

// Returns object's name
FText ABasicInformativeObject::GetName_Implementation()
{
	return Name;
}
// Returns basic infomation about the object
FText ABasicInformativeObject::GetBasicInfo_Implementation()
{
	return BasicInfo;
}

// Sets default values
ABasicInformativeObject::ABasicInformativeObject()
{
	// Set this actor to NOT call Tick() every frame
	PrimaryActorTick.bCanEverTick = false;
	// TODO enable ticks?
}

// Called when the game starts or when spawned
void ABasicInformativeObject::BeginPlay()
{
	Super::BeginPlay();
	
	// TODO
}

// Called every frame
void ABasicInformativeObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO enable ticks?
}