// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameHUD.generated.h"

// Game's UI
UCLASS()
class DARKLAB_API UGameHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// Test it yay
	UFUNCTION(BlueprintCallable, Category = "Game HUD")
	float TestFloatReturner();
};