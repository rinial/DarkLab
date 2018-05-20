// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicPickupableObject.h"
#include "Doorcard.generated.h"

// A card that can open doors, for now cards can be used many times but all of them are stored in character when he picks them up (as all pickupable objects should)
UCLASS()
class DARKLAB_API ADoorcard : public ABasicPickupableObject
{
	GENERATED_BODY()

public:
	// Sets the color
	UFUNCTION(BlueprintCallable, Category = "Doorcard")
	void SetColor(FLinearColor color);
	// Returns the color
	UFUNCTION(BlueprintCallable, Category = "Doorcard")
	FLinearColor GetColor();

private:
	// Update's the color of the doorcard mesh
	UFUNCTION()
	void UpdateMeshColor(FLinearColor color);
	
protected:
	// Card's direction and root object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Doorcard: Components")
	class UArrowComponent* Direction;

	// The card itself
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Doorcard: Components")
	class UStaticMeshComponent* Card;

protected:
	// The color of the card
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Doorcard")
	FLinearColor Color = FLinearColor::FromSRGBColor(FColor(30, 144, 239));

public:
	// Sets default values
	ADoorcard();
};