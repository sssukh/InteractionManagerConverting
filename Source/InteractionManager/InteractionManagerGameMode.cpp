// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractionManagerGameMode.h"
#include "InteractionManagerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AInteractionManagerGameMode::AInteractionManagerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
