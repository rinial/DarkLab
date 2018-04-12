// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

UCLASS()
class DARKLAB_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	// Sets default values
	AMainPlayerController();
	
protected:
	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	// Cotntrol where the character goes
	void MoveUp(float Value);
	void MoveRight(float Value);

	// Control where the character looks
	void LookWithStick();
	void LookWithMouse();
};