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
	// While true rotation is controled with mouse
	bool bLookWithMouse = true;
	// Character under control
	class AMainCharacter* Character;

	// While true character can be moved and rotated and he can use stuff
	bool bCharacterActive = true;

	// Cotntrol where the character goes
	void MoveUp(const float value);
	void MoveRight(const float value);

	// Control where the character looks
	void LookWithMouse();
	void LookWithStick();

	// Makes the character use something he has equiped
	void UseEquiped();
	// Makes the character avtivate smth near him
	void Activate();

	// Show/Hide menu
	void ShowHideMenu();

	// The number of character's 'lives'
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Character Controller", meta = (AllowPrivateAccess = "true"))
	int Lives = 3;

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