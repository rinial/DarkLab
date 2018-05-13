// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Equipable.h"
#include "Usable.h"
#include "Activatable.h"
#include "Informative.h"
#include "MainPlayerController.h"
#include "MainGameMode.h"
// For on screen debug
#include "EngineGlobals.h"
#include "Engine/Engine.h"

// Movement functions
void AMainCharacter::Move(FVector direction, const float value)
{
	direction.Normalize();
	FRotator rotation = GetActorRotation();
	FVector lookDirection = rotation.Vector();
	FVector v2 = direction * (value >= 0 ? 1 : -1);
	// Angle between applied movement and view direction
	float angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(lookDirection, v2)));

	// Movement is slower when looking in the opposite direction of movement
	AddMovementInput(direction, value * (1.0f - (1.0f - BackMoveCoeff) * angle / 180.0f));
}
void AMainCharacter::MoveUp(const float value)
{
	Move(FVector(1.0f, 0.0f, 0.0f), value);
}
void AMainCharacter::MoveRight(const float value)
{
	Move(FVector(0.0f, 1.0f, 0.0f), value);
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

	IUsable* toUse = Cast<IUsable>(EquipedObject->_getUObject());
	if (toUse)
		toUse->Execute_Use(EquipedObject->_getUObject());
}

// Activates nearby object on scene
void AMainCharacter::Activate()
{
	TScriptInterface<IActivatable> toActivate = GetActivatable();
	if (!toActivate)
		return;
	toActivate->Execute_Activate(toActivate->_getUObject(), this);
}

// Happens when something 'damages' the character
void AMainCharacter::Disable()
{
	UE_LOG(LogTemp, Warning, TEXT("Got disabled"));

	bIsDisabled = true;

	// TODO destroy instead?
	this->SetActorEnableCollision(false);
	PrimaryActorTick.SetTickFunctionEnable(false);

	// Tell the controller that we got disabled
	AMainPlayerController* controller = Cast<AMainPlayerController>(GetController());
	if (controller)
		controller->OnDisabled();

	// TODO Destroy?
}

// Used for the activator's collision overlaps
void AMainCharacter::OnActivatorBeginOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// We don't count our currently equiped object
	UObject* otherObject = Cast<UObject>(OtherActor);
	if (EquipedObject && otherObject == EquipedObject->_getUObject())
		return;

	IActivatable* activatable = Cast<IActivatable>(OtherActor);
	if (!activatable || !activatable->Execute_IsActivatableDirectly(otherObject))
		return;

	// UE_LOG(LogTemp, Warning, TEXT("Found activatable"));

	ActivatableObjects.AddUnique(OtherActor);
}
void AMainCharacter::OnActivatorEndOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	// UE_LOG(LogTemp, Warning, TEXT("Lost activatable"));

	ActivatableObjects.Remove(OtherActor);
}

// Returns an activatable object in front of the character
TScriptInterface<class IActivatable> AMainCharacter::GetActivatable()
{
	if(ActivatableObjects.Num() == 0)
		return nullptr;

	TScriptInterface<class IActivatable> currentSelected = nullptr;
	float currentValue = 0; // value used to determine priority
	for (TScriptInterface<class IActivatable> activatable : ActivatableObjects)
	{
		AActor* actor = Cast<AActor>(activatable->_getUObject());
		// We check if actor can be seen before its actually activatable
		// The check is done with Z increased so that there are no conflicts with floor
		if (!GameMode->CanSee(this, actor, actor->GetActorLocation() + FVector(0, 0, 30), true)) // TODO disable debug here
			continue;

		FVector location = actor->GetActorLocation();
		FVector toActiv = location - GetActorLocation();
		float distance = toActiv.Size2D(); // ~30 - ~150
		toActiv.Normalize();
		FVector lookDirection = GetActorRotation().Vector();
		float angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(lookDirection, toActiv))); // 0 - ~80
		float temp = distance / 30.0f + angle / 10.0f;
		if (temp < currentValue || currentSelected == nullptr)
		{
			currentValue = temp;
			currentSelected = activatable;
		}
	}
	return currentSelected;
}

// TODO move somewhere, its not bound to character
// Sets outline for an actor
void AMainCharacter::SetOutline(UObject * object, bool showOutline) const
{
	if (!object)
		return;

	AActor* actor = Cast<AActor>(object);

	if (!actor)
		return;

	SetOutline(actor, showOutline);
}
void AMainCharacter::SetOutline(AActor* actor, bool showOutline) const
{
	if (!actor)
		return;

	TArray<UStaticMeshComponent*> components;
	actor->GetComponents<UStaticMeshComponent>(components);
	for (UStaticMeshComponent* component : components)
		component->SetRenderCustomDepth(showOutline);
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

	// Create an activator
	Activator = CreateDefaultSubobject<UBoxComponent>(TEXT("Activator"));
	Activator->SetupAttachment(RootComponent);
	Activator->OnComponentBeginOverlap.AddDynamic(this, &AMainCharacter::OnActivatorBeginOverlap);
	Activator->OnComponentEndOverlap.AddDynamic(this, &AMainCharacter::OnActivatorEndOverlap);

 	// Set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
}

// Called every frame
void AMainCharacter::Tick(const float deltaTime)
{
	Super::Tick(deltaTime);

	// TODO move to a separate function
	TScriptInterface<IActivatable> newSelectedForActivation = GetActivatable();
	if (SelectedForActivation != newSelectedForActivation)
	{
		if (SelectedForActivation)
			SetOutline(SelectedForActivation->_getUObject(), false);
		if (newSelectedForActivation)
			SetOutline(newSelectedForActivation->_getUObject(), true);
		SelectedForActivation = newSelectedForActivation;
	}

	// TODO delete
	// We check the light level
	// GameMode->GetLightingAmount(this, true);

	// TODO delete later: used for debug
	if (GEngine)
	{
		TArray<IInformative*> informativeObjects;
		TArray<FString> informativeNames;

		// Activatable in front
		TScriptInterface<IActivatable> activatable = GetActivatable();
		informativeObjects.Add(activatable ? Cast<IInformative>(activatable->_getUObject()) : nullptr);
		informativeNames.Add("Activatable");

		// Currently equipped
		informativeObjects.Add(EquipedObject ? Cast<IInformative>(EquipedObject->_getUObject()) : nullptr);
		informativeNames.Add("Equipped");

		for (int i = 0; i < informativeObjects.Num(); ++i)
		{
			IInformative* informative = informativeObjects[i];
			FString name = informativeNames[i];

			if (informative)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("    %s"), *(informative->Execute_GetBasicInfo(informative->_getUObject())).ToString()), true);
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("%s: %s"), *name, *(informative->Execute_GetName(informative->_getUObject())).ToString()), true);
			}
			else
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("%s: %s"), *name, TEXT("None")), true);
		}

		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT(""), true);
	}
}