// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"

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

// TODO
// Called on loss
void AMainCharacter::OnLoss()
{
	UE_LOG(LogTemp, Warning, TEXT("You lost!"));
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
	
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}