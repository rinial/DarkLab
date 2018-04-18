// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Equipable.h"
#include "Usable.h"
#include "MainPlayerController.h"
// TODO delete later?
#include "Flashlight.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Engine/PointLight.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Components/PointLightComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectIterator.h"

// Movement functions
void AMainCharacter::MoveUp(const float value)
{
	FRotator rotation = GetActorRotation();
	FVector direction = rotation.Vector();
	FVector v2 = FVector(1.0f, 0.0f, 0.0f) * value;
	// Angle between applied movement and view direction
	float angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(direction, v2)));
	// UE_LOG(LogTemp, Warning, TEXT("%f"), angle);

	AddMovementInput(FVector(1.0f - (1.0f - BackMoveCoeff) * angle / 180.0f, 0.0f, 0.0f), value);
}
void AMainCharacter::MoveRight(const float value)
{
	FRotator rotation = GetActorRotation();
	FVector direction = rotation.Vector();
	FVector v2 = FVector(0.0f, 1.0f, 0.0f) * value;
	// Angle between applied movement and view direction
	float angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(direction, v2)));
	// UE_LOG(LogTemp, Warning, TEXT("%f"), angle);

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

// TODO make it more generic, move somewhere
// Returns the light level on the position
float AMainCharacter::GetLightingAmount()
{
	FVector Loc = GetActorLocation();
	// Checks head
	Loc += FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	// TODO should somehow make it ignore equiped stuff?
	FCollisionQueryParams Params = FCollisionQueryParams(FName(TEXT("LightTrace")), true, this);
	float Result = 0.0f;

	TArray<UPointLightComponent*> PointLights;
	UWorld* YourGameWorld = GetWorld();
	for (TObjectIterator<UPointLightComponent> Itr; Itr; ++Itr)
	{
		//World Check
		if (Itr->GetWorld() != YourGameWorld)
		{
			continue;
		}

		PointLights.Add(*Itr);
	}

	for (UPointLightComponent* LightComp : PointLights)
	{
		FVector End = LightComp->GetComponentLocation();
		float Distance = FVector::Dist(Loc, End);
		float LightRadius = LightComp->AttenuationRadius;
		bool bHit = GetWorld()->LineTraceTestByChannel(Loc, End, ECC_Visibility, Params);

		//UE_LOG(LogTemp, Warning, TEXT("wow light"));
		if (Distance <= LightRadius && !bHit)
		{
			float temp = FMath::Pow(FMath::Max(0.0f, 1.0f - (Distance / LightRadius)), (LightComp->LightFalloffExponent + 1)) * (LightRadius * 1.25);
			temp = FMath::Clamp(temp, 0.0f, 1.0f);
			//UE_LOG(LogTemp, Warning, TEXT("%f"), temp);
			// It always counts the brightest light
			if (temp > Result)
				Result = temp;
		}
	}
	
	if (Result > 1.0f)
		Result = 1.0f;

	//UE_LOG(LogTemp, Warning, TEXT("Final %f"), Result);
	return Result;
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

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	PrimaryActorTick.bCanEverTick = false;

	// Tell the controller that we lost
	AMainPlayerController* controller = Cast<AMainPlayerController>(GetController());
	if (controller)
		controller->OnLoss();

	// use Delay for some animations to play

	// GetWorld()->GetAuthGameMode()->RestartPlayerAtPlayerStart();

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
	// delete
	GetLightingAmount();
}