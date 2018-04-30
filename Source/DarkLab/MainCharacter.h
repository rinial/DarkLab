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
	void MoveUp(const float value);
	void MoveRight(const float value);
	void Look(const FVector direction);

	// Other controls
	void UseEquiped();
	void Activate();

	// Happens when something 'damages' the character
	UFUNCTION(BlueprintCallable, Category = "Main Character")
	void Disable();

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

private:
	// A reference to the game mode
	UPROPERTY()
	class AMainGameMode* GameMode;

	// TODO delete later
	// Move spawns to gamemode or somewhere else
	TSubclassOf<class AFlashlight> MyFlashlightBP;

public:
	// Some equiped object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character")
	TScriptInterface<class IEquipable> EquipedObject;

	// Is true when a character loses a life. Used for "death" animation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character")
	bool bIsDisabled = false;

public:
	// Sets default values
	AMainCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(const float deltaTime) override;
};