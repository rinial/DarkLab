// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"

// Movement functions
void AMainCharacter::MoveUp(const float value)
{
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), value);
}
void AMainCharacter::MoveRight(const float value)
{
	AddMovementInput(FVector(0.0f, 1.0f, 0.0f), value);
}
void AMainCharacter::Look(const FVector direction)
{
	SetActorRotation(direction.Rotation());
}

// Takes one 'life' and calls CalculateLoss
void AMainCharacter::TakeLife()
{
	if (Lives > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Died once"));
		--Lives;
	}
	CalculateLoss();
}

// Checks for the loss
void AMainCharacter::CalculateLoss()
{
	if (Lives <= 0)
		OnLoss();
}
// Called on loss
void AMainCharacter::OnLoss()
{
	UE_LOG(LogTemp, Warning, TEXT("You lost!"));

	// TODO
}

// Sets default values
AMainCharacter::AMainCharacter()
{
	// Create a camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	// Create a camera
	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

 	// Set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// TODO
}

// Called every frame
void AMainCharacter::Tick(const float deltaTime)
{
	Super::Tick(deltaTime);

	// TODO
}