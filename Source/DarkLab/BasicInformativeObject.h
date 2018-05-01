// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Informative.h"
#include "BasicInformativeObject.generated.h"

// Represents objects with physical representation that can provide information about themselves
UCLASS()
class DARKLAB_API ABasicInformativeObject : public AActor, public IInformative
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};