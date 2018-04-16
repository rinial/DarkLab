// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Equipable.h"
#include "Usable.h"
// TODO delete later?
#include "Flashlight.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

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

// Uses equiped object if it is IUsable
void AMainCharacter::UseEquiped()
{
	if (!EquipedObject)
		return;

	IUsable* toUse = Cast<IUsable>(EquipedObject);
	if (toUse)
		toUse->Execute_Use(Cast<UObject>(EquipedObject));
}

// Activates nearby object on scene
void AMainCharacter::Activate()
{
	UE_LOG(LogTemp, Warning, TEXT("Activated"));

	// TODO
}

// Takes one 'life' and calls CalculateLoss
void AMainCharacter::TakeLife()
{
	if (Lives > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Died once"));
		--Lives;
		bIsDisabled = true;
	}
	CalculateLoss();
}

// Checks for the loss
void AMainCharacter::CalculateLoss()
{
	if (Lives <= 0)
		OnLoss();
	// TODO if not lost, make a delay, after that change bIsDisabled to false and respawn
}
// Called on loss
void AMainCharacter::OnLoss()
{
	UE_LOG(LogTemp, Warning, TEXT("You lost!"));

	APlayerController* controller = Cast<APlayerController>(GetController());
	if(controller)
		DisableInput(controller);

	// use Delay for some animations to play

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

	// TODO delete later: we shouldn't find blueprints from character
	static ConstructorHelpers::FObjectFinder<UBlueprint> flashlightBP(TEXT("Blueprint'/Game/Blueprints/FlashlightBP.FlashlightBP'"));
	if (flashlightBP.Object)
		MyFlashlightBP = (UClass*)flashlightBP.Object->GeneratedClass;

 	// Set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// TODO delete later: we shouldn't spawn objects from character
	AFlashlight* flashlight = GetWorld()->SpawnActor<AFlashlight>(MyFlashlightBP, GetActorLocation(), GetActorRotation());
	UE_LOG(LogTemp, Warning, TEXT("Spawned a flashlight"));
	IEquipable* toEquip = Cast<IEquipable>(flashlight);
	if (toEquip)
		toEquip->Execute_Equip(flashlight, this);
}

// Called every frame
void AMainCharacter::Tick(const float deltaTime)
{
	Super::Tick(deltaTime);

	// TODO
}