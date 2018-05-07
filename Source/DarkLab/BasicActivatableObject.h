// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicInformativeObject.h"
#include "Activatable.h"
#include "BasicActivatableObject.generated.h"

// Represents objects with physical representation that can be activated
UCLASS()
class DARKLAB_API ABasicActivatableObject : public ABasicInformativeObject, public IActivatable
{
	GENERATED_BODY()
	
public:
	// Called when the object is activated by character
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Activatable")
	void Activate(AMainCharacter* character);
	virtual void Activate_Implementation(AMainCharacter* character) override;
	
	// Called when the object is activated indirectly
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Activatable")
	void ActivateIndirectly();
	virtual void ActivateIndirectly_Implementation() override;

	// Returns true if activatable directly
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Activatable")
	bool IsActivatableDirectly();
	virtual bool IsActivatableDirectly_Implementation() override;

	// Returns true if activatable indirectly
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Activatable")
	bool IsActivatableIndirectly();
	virtual bool IsActivatableIndirectly_Implementation() override;

protected:
	// Called when the object is activated (should always be overridden)
	UFUNCTION(BlueprintCallable, Category = "Basic Activatable")
	virtual void ActivateObject(AMainCharacter* character);

	// Controls how the object can be activated
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Basic Activatable")
	bool bActivatableDirectly = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Basic Activatable")
	bool bActivatableIndirectly = true;
};