// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "DarkLabGameMode.h"
#include "DarkLabPlayerController.h"
#include "DarkLabCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADarkLabGameMode::ADarkLabGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ADarkLabPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}