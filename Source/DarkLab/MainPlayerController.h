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

	// Cotntrol where the character goes
	void MoveUp(const float value);
	void MoveRight(const float value);

	// Control where the character looks
	void LookWithMouse();
	void LookWithStick();
	
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