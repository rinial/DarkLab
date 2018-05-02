// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicPlaceableObject.h"
#include "Informative.h"
#include "BasicInformativeObject.generated.h"

// Represents objects with physical representation that can provide information about themselves
UCLASS()
class DARKLAB_API ABasicInformativeObject : public ABasicPlaceableObject, public IInformative
{
	GENERATED_BODY()

public:
	// Returns object's name
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Informative")
	FText GetName();
	virtual FText GetName_Implementation() override;

	// Returns basic infomation about the object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Informative")
	FText GetBasicInfo();
	virtual FText GetBasicInfo_Implementation() override;

protected:
	UPROPERTY()
	FText Name = FText();

	UPROPERTY()
	FText BasicInfo = FText();
	
public:	
	// Sets default values
	ABasicInformativeObject();
};