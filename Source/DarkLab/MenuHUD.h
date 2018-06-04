// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuHUD.generated.h"

// Menu's UI
UCLASS()
class DARKLAB_API UMenuHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// Shows overlay during transition
	UFUNCTION(BlueprintImplementableEvent, Category = "Menu HUD")
	void OnChangeMap();
	UFUNCTION(BlueprintImplementableEvent, Category = "Menu HUD")
	void OnExit();
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Menu HUD")
	class AMainPlayerController* Controller;
};