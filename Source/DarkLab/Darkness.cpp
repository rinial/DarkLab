// Fill out your copyright notice in the Description page of Project Settings.

#include "Darkness.h"

// Sets default values
ADarkness::ADarkness()
{
	// TODO

 	// Set this pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADarkness::BeginPlay()
{
	Super::BeginPlay();

	// TODO
}

// Called every frame
void ADarkness::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO
}

// Called to bind functionality to input
void ADarkness::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// TODO delete?
}