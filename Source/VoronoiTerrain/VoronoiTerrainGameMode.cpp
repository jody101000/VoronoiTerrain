// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoronoiTerrainGameMode.h"
#include "VoronoiTerrainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "MovingPlatformManager.h"

AVoronoiTerrainGameMode::AVoronoiTerrainGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

// AActor* AVoronoiTerrainGameMode::ChoosePlayerStart_Implementation(AController* Player, const FString& IncomingName)
// {
// 	
// 	FVector FixedSpawnLocation = FVector(0.0f, 0.0f, 100.0f); // Example coordinates
// 	FRotator FixedSpawnRotation = FRotator(0.0f, 0.0f, 0.0f); // Example rotation
// 	GetWorld()->SpawnActor<APlayerStart>(DefaultPawnClass, FixedSpawnLocation, FixedSpawnRotation);
// 	
// 	return Super::ChoosePlayerStart_Implementation(Player);
// }

