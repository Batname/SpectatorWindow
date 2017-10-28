// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SpectatorWindowGameMode.h"
#include "SpectatorWindowHUD.h"
#include "SpectatorWindowCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASpectatorWindowGameMode::ASpectatorWindowGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ASpectatorWindowHUD::StaticClass();
}
