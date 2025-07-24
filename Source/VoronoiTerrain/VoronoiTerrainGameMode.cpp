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
// 	TArray<AActor*> PlayerStarts;
// 	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
// 	
// 	if (PlayerStarts.Num() > 0)
// 	{
// 		PlayerStarts[0]->SetActorLocation()
// 		return Cast<APlayerStart>(PlayerStarts[0]);
// 	}
//
// 	return Super::ChoosePlayerStart_Implementation(Player);
// }

