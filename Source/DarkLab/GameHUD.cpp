// Fill out your copyright notice in the Description page of Project Settings.

#include "GameHUD.h"
#include "MainCharacter.h"
#include "Lighter.h"
#include "Flashlight.h"

// Get items info
//FText UGameHUD::GetItem1Name()
//{
//	// TODO
//	return FText();
//}
FText UGameHUD::GetItem1Info()
{
	if (Character->LighterIndex < 0)
		return FText();
	
	TScriptInterface<IPickupable> obj = Character->Inventory[Character->LighterIndex];
	int power = FMath::CeilToInt(Cast<ALighter>(obj->_getUObject())->PowerLevel * 100.f);

	return FText::FromString("Gas: " + FString::FromInt(power) + "%");
}
//FText UGameHUD::GetItem2Name()
//{
//	// TODO
//	return FText();
//}
FText UGameHUD::GetItem2Info()
{
	if (Character->FlashlightIndex < 0)
		return FText();

	TScriptInterface<IPickupable> obj = Character->Inventory[Character->FlashlightIndex];
	int power = FMath::CeilToInt(Cast<AFlashlight>(obj->_getUObject())->PowerLevel * 100.f);

	return FText::FromString("Power: " + FString::FromInt(power) + "%");
}

// Get activatable info
FText UGameHUD::GetActivatableName()
{
	TScriptInterface<IActivatable> activatable = Character->GetActivatable();
	if (!activatable)
		return FText();
	
	IInformative* informative = Cast<IInformative>(activatable->_getUObject());
	if (!informative)
		return FText();

	return informative->Execute_GetName(informative->_getUObject());
}
FText UGameHUD::GetActivatableInfo()
{
	TScriptInterface<IActivatable> activatable = Character->GetActivatable();
	if (!activatable)
		return FText();

	IInformative* informative = Cast<IInformative>(activatable->_getUObject());
	if (!informative)
		return FText();

	return informative->Execute_GetBasicInfo(informative->_getUObject());
}