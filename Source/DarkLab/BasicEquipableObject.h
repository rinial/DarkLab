// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Equipable.h"
#include "Activatable.h"
#include "BasicEquipableObject.generated.h"

// Represents objects with physical representation that can be equiped by the main character
UCLASS()
class DARKLAB_API ABasicEquipableObject : public AActor, public IEquipable, public IActivatable
{
	GENERATED_BODY()

public:
	// Called when the object is to be equiped
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipable")
	void Equip(AMainCharacter* character, FName location);
	virtual void Equip_Implementation(AMainCharacter* character, FName location) override;
	
	// Called when the object is to be unequiped
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipable")
	void Unequip(AMainCharacter* character);
	virtual void Unequip_Implementation(AMainCharacter* character) override;

	// Called when the object is activated (taken in the laboratory)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Activatable")
		void Activate(AMainCharacter* character);
	virtual void Activate_Implementation(AMainCharacter* character) override;
	
public:	
	// Sets default values
	ABasicEquipableObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(const float DeltaTime) override;
};