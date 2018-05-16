// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

// Used to avoid engine bug in UCharacterMovementComponent::PhysWalking
UCLASS()
class DARKLAB_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
protected:
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;
};