// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoronoiTerrainGameMode.h"
#include "VoronoiTerrainCharacter.h"
#include "UObject/ConstructorHelpers.h"

AVoronoiTerrainGameMode::AVoronoiTerrainGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
