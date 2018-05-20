// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicPlaceableObject.h"
#include "ExitVolume.generated.h"

// The exit player is trying to reach
UCLASS()
class DARKLAB_API AExitVolume : public ABasicPlaceableObject
{
	GENERATED_BODY()

public:
	// Activates volume's light
	UFUNCTION(BlueprintCallable, Category = "Exit")
	void ActivateLight();

	// TODO let some interface define it?
	// Resets to initial state
	UFUNCTION(BlueprintCallable, Category = "Exit")
	void Reset();

public:
	// Used for the collision overlaps
	UFUNCTION(BlueprintCallable, Category = "Exit: Overlap")
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Exit: Overlap")
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
protected:
	// Volume's direction and root object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exit: Components")
	class UArrowComponent* Direction;
	
	// The box collision of the volume
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exit: Components")
	class UBoxComponent* Collision;
	
	// The volume's light
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exit: Components")
	class UPointLightComponent* Light;

public:
	// Sets default values
	AExitVolume();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};