// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Equipable.h"
#include "Usable.h"
#include "MainPlayerController.h"
#include "MainGameMode.h"
// TODO delete later?
#include "Flashlight.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/CapsuleComponent.h"

// Movement functions
void AMainCharacter::MoveUp(const float value)
{
	FRotator rotation = GetActorRotation();
	FVector direction = rotation.Vector();
	FVector v2 = FVector(1.0f, 0.0f, 0.0f) * value;
	// Angle between applied movement and view direction
	float angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(direction, v2)));

	// Movement is slower when looking in the opposite direction of movement
	AddMovementInput(FVector(1.0f - (1.0f - BackMoveCoeff) * angle / 180.0f, 0.0f, 0.0f), value);
}
void AMainCharacter::MoveRight(const float value)
{
	FRotator rotation = GetActorRotation();
	FVector direction = rotation.Vector();
	FVector v2 = FVector(0.0f, 1.0f, 0.0f) * value;
	// Angle between applied movement and view direction
	float angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(direction, v2)));

	// Movement is slower when looking in the opposite direction of movement
	AddMovementInput(FVector(0.0f, 1.0f - (1.0f - BackMoveCoeff) * angle / 180.0f, 0.0f), value);
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

// Happens when something 'damages' the character
void AMainCharacter::Disable()
{
	UE_LOG(LogTemp, Warning, TEXT("Got disabled"));

	bIsDisabled = true;

	// TODO destroy instead?
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	PrimaryActorTick.SetTickFunctionEnable(false);

	// Tell the controller that we got disabled
	AMainPlayerController* controller = Cast<AMainPlayerController>(GetController());
	if (controller)
		controller->OnDisabled();

	// TODO Destroy?
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

	GameMode = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
	
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

	// TODO delete
	// We check the light level
	// GameMode->GetLightingAmount(this, true);
}