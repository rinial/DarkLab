// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

// The main character of the game
UCLASS(Blueprintable)
class DARKLAB_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Movement 
	void Move(FVector direction, const float value);
	void MoveUp(const float value);
	void MoveRight(const float value);
	void Look(const FVector direction);

	// Other controls
	void UseEquiped();
	void Activate();

	// Happens when something 'damages' the character
	UFUNCTION(BlueprintCallable, Category = "Main Character")
	void Disable();
	// Called from controller when it's time to enable character again
	UFUNCTION(BlueprintCallable, Category = "Main Character")
	void Enable();

protected:
	// The coefficient for the movement backwards
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Character: Movement")
	float BackMoveCoeff = 0.6f;

	// Camera boom positioning the camera above the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character: Components")
	class USpringArmComponent* CameraBoom;

	// Top down camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character: Components")
	class UCameraComponent* TopDownCamera;

	// Activator component checking for activatable objects
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character: Components")
	class UBoxComponent* Activator;

public:
	// Used for the activator's collision overlaps
	UFUNCTION(BlueprintCallable, Category = "Main Character: Overlap")
	void OnActivatorBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Main Character: Overlap")
	void OnActivatorEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Returns an activatable object in front of the character
	UFUNCTION(BlueprintCallable, Category = "Main Character")
	TScriptInterface<class IActivatable> GetActivatable();

	// TODO move somewhere, its not bound to character
	// Sets outline for an actor
	void SetOutline(UObject* object, bool showOutline) const;
	void SetOutline(AActor* actor, bool showOutline) const;

private:
	// A reference to the game mode
	UPROPERTY()
	class AMainGameMode* GameMode;

public:
	// Is true when a character loses a life. Used for "death" animation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character")
	bool bIsDisabled = false;

	// Some equiped object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character")
	TScriptInterface<class IEquipable> EquipedObject;

	// Objects in character's inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character")
	TArray<TScriptInterface<class IPickupable>> Inventory;

	// Objects currently inside the activator volume
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character")
	TArray<TScriptInterface<class IActivatable>> ActivatableObjects;

protected:
	// Object currently selected for activation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character")
	TScriptInterface<IActivatable> SelectedForActivation;

public:
	// Sets default values
	AMainCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(const float deltaTime) override;
};