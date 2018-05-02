// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Deactivatable.h"
#include "BasicDeactivatableObject.generated.h"

// Represents objects with physical representation that can be activated/deactivated
UCLASS()
class DARKLAB_API ABasicDeactivatableObject : public AActor, public IDeactivatable
{
	GENERATED_BODY()

public:
	// Activates/deactivates the object (usually for pooling)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Deactivatable")
	void SetActive(bool active);
	virtual void SetActive_Implementation(bool active) override;

	// Returns true if the object is active
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Deactivatable")
	bool IsActive();
	virtual bool IsActive_Implementation() override;

protected:
	UPROPERTY()
	bool bIsActive = true;

	UPROPERTY()
	bool bDefaultTickEnabled = false;
	
public:	
	// Sets default values
	ABasicDeactivatableObject();
};