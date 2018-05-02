// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicInformativeObject.h"
#include "Activatable.h"
#include "BasicDoor.generated.h"

// A openable simple door
UCLASS()
class DARKLAB_API ABasicDoor : public ABasicInformativeObject, public IActivatable
{
	GENERATED_BODY()

public:
	// Called when the object is activated
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Activatable")
	void Activate(AMainCharacter* character);
	virtual void Activate_Implementation(AMainCharacter* character) override;
	
protected:
	// Door's direction and root object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door: Components")
	class UArrowComponent* Direction;

	// Door's frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door: Components")
	class UStaticMeshComponent* DoorFrame;

	// Timeline, driving the door
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door: Components")
	class UTimelineComponent* DoorDriver;

	// The color of panels on the door
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door: Components")
	FLinearColor DoorColor = FLinearColor::White;
	
public:
	// Sets default values
	ABasicDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};