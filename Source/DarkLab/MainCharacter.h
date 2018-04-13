// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "MainCharacter.generated.h"

UCLASS(Blueprintable)
class DARKLAB_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Movement 
	void MoveUp(const float value);
	void MoveRight(const float value);
	void Look(const FVector direction);

	// Takes one 'life' and calls CalculateLoss
	UFUNCTION(BlueprintCallable, Category = "Main Character")
	void TakeLife();

private:
	// Checks for the loss
	void CalculateLoss();
	// Called on loss
	void OnLoss();

	// The number of character's 'lives'
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Character", meta = (AllowPrivateAccess = "true"))
	int Lives = 1;

	// Camera boom positioning the camera above the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	// Top down camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Character", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* TopDownCamera;

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