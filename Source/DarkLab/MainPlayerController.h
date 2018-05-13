// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

// Handles controls
UCLASS()
class DARKLAB_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	// Character under control
	UPROPERTY()
	class AMainCharacter* MainCharacter;

protected:
	// While true rotation is controled with mouse
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Player Controller")
	bool bLookWithMouse = true;

	// The number of character's 'lives'
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Player Controller")
	int Lives = 3;

	// The time between a character 'death' and respawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Player Controller")
	float RespawnDelay = 4.0f;

private:
	// Stores last mouse position to check when it changes
	// GetInputMouseDelta doesn't really work as it should
	FVector2D LastMousePosition;

protected:
	// Cotntrol where the character goes
	UFUNCTION()
	void MoveUp(const float value);
	UFUNCTION()
	void MoveRight(const float value);

	// Control where the character looks
	UFUNCTION()
	void LookWithMouse();
	UFUNCTION()
	void LookWithStick();

	// Makes the character use something he has equiped
	UFUNCTION()
	void UseEquiped();
	// Makes the character avtivate smth near him
	UFUNCTION()
	void Activate();

	// Show/Hide menu
	UFUNCTION()
	void ShowHideMenu();

	// Resets map, only used for debug
	UFUNCTION()
	void ResetMap();

public:
	// Called from the main character. Takes one 'life' and calls CalculateLoss
	void OnDisabled();

private:
	// Checks for the loss and calls OnLoss
	void CalculateLoss();
	// Called when disabled and no 'lives' are left
	void OnLoss();
	// Respawns the character
	void Enable();
	
public:
	// Sets default values
	AMainPlayerController();
	
protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
	// Sets controls
	void SetupInputComponent() override;
	// Called every frame
	void PlayerTick(const float deltaTime) override;
};