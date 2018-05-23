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
	// Call to show item2 panel
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void ShowItem2Panel();
	// Call to outline items
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void OutlineItem(int itemNum, bool show);

	// Get items info
	/*UFUNCTION(BlueprintCallable, Category = "Game HUD")
	FText GetItem1Name();*/
	UFUNCTION(BlueprintCallable, Category = "Game HUD")
	FText GetItem1Info();
	/*UFUNCTION(BlueprintCallable, Category = "Game HUD")
	FText GetItem2Name();*/
	UFUNCTION(BlueprintCallable, Category = "Game HUD")
	FText GetItem2Info();

	// Get activatable info
	UFUNCTION(BlueprintCallable, Category = "Game HUD")
	FText GetActivatableName();
	UFUNCTION(BlueprintCallable, Category = "Game HUD")
	FText GetActivatableInfo();
	// Shows or hides activatable panel
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void ShowHideActivatablePanel(bool show);

	// Shows new card of color
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void ShowDoorcard(FLinearColor color);

	// Call to show game end messages
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void ShowVictoryMessage();
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void ShowLossMessage();

	// Shows or hides a warning
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void ShowHideWarning(bool show, const FText& warning); // !!

	// Shows or hides menu
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void ShowHideMenu(bool show);
	// Shows or hides help
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void ShowHideHelp(bool show);

	// Shows overlay during transition
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void OnRestart();
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void OnChangeMap();
	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void OnExit();

public:
	UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
	class AMainPlayerController* Controller;
	UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
	class AMainCharacter* Character;
	UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
	class AMainGameMode* GameMode;
};