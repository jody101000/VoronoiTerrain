// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformSystem/PlatformPathManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "FortuneAlgorithm/FortuneAlgorithm.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/Material.h"
#include "Actors/LevelGoal.h"
#include "Actors/ResourcePickup.h"

APlatformPathManager::APlatformPathManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	PlatformTypeManager = CreateDefaultSubobject<UPlatformTypeManager>(TEXT("PlatformTypeManager"));

	PlatformTypeWeights.Add(EPlatformType::Standard, 0.7f);
	PlatformTypeWeights.Add(EPlatformType::Moving, 0.3f);

	PlatformMeshesByType.Add(EPlatformType::Standard, FPlatformMeshArray());
	PlatformMeshesByType.Add(EPlatformType::Moving, FPlatformMeshArray());
	
	ResourceTypeWeights.Add(EResourceType::None, 0.3f);
	ResourceTypeWeights.Add(EResourceType::Resource1, 0.3f);
	ResourceTypeWeights.Add(EResourceType::Resource2, 0.25f);
	ResourceTypeWeights.Add(EResourceType::Resource3, 0.15f);
}

void APlatformPathManager::BeginPlay()
{
	Super::BeginPlay();

	GeneratePlatformSpiralPositions();
	CreatePlatforms();
}

void APlatformPathManager::OnConstruction(const FTransform& Transform)
{
	FlushPersistentDebugLines(GetWorld());

	// ToDo: Each Type has to have a mesh selected
	if (!PlatformMeshesByType.Contains(EPlatformType::Standard) ||
		PlatformMeshesByType[EPlatformType::Standard].Meshes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No meshes assigned for Standard platform type"));
		return;
	}

	GeneratePlatformSpiralPositions();

	if (ShowDebugCircles)
	{
		for (const auto& PlatformInfo : PlacedPlatforms)
		{
			EPlatformType Type = PlatformInfo.Type;
			FColor DebugColor = FColor::White;

			switch (Type)
			{
			case EPlatformType::Standard: DebugColor = FColor::White; break;
			case EPlatformType::Moving: DebugColor = FColor::Yellow; break;
			}

			DrawDebugCircle(GetWorld(), PlatformInfo.Position + GetActorLocation(),
				PlatformSize, 24, DebugColor, true, -1, 0, 2,
				FVector(0, 1, 0), FVector(1, 0, 0), false);
		}
	}
}


void APlatformPathManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void APlatformPathManager::CreatePlatforms()
{
	DestroyPlatforms();

	int CreatedPlatforms = 0;

	for (int i = 0; i < PlatformCount; i++)
	{
		if (!PlacedPlatforms.IsValidIndex(i))
		{
			continue;
		}

		const FPlacedPlatformInfo& PlatformInfo = PlacedPlatforms[i];

		FVector WorldPosition = GetActorLocation() + PlatformInfo.Position;
		EPlatformType SelectedType = PlatformInfo.Type;
		UStaticMesh* SelectedMesh = PlatformInfo.Mesh;

		FVector MeshSize = SelectedMesh->GetBounds().GetBox().GetSize();
		float MaxSize = FMath::Max3(MeshSize.X, MeshSize.Y, MeshSize.Z);
		float Scale = PlatformSize / MaxSize * 2.0f;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Name = *FString::Printf(TEXT("Platform_%d_%s_%s"), CreatedPlatforms,
			*UEnum::GetValueAsString(SelectedType), *GetName());
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		
		APlatformComponent* NewPlatform = GetWorld()->SpawnActorDeferred<APlatformComponent>(
			APlatformComponent::StaticClass(),
			FTransform(FRotator::ZeroRotator, WorldPosition, FVector(Scale)),
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding
		);

		if (NewPlatform)
		{
			NewPlatform->PreInitializePlatform(SelectedType, SelectedMesh, CreatedPlatforms);

			UGameplayStatics::FinishSpawningActor(NewPlatform,
				FTransform(FRotator::ZeroRotator, WorldPosition, FVector(Scale)));

			NewPlatform->PostInitializePlatform(WorldPosition, FRotator::ZeroRotator, Scale);
			NewPlatform->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

			PlatformComponents.Add(NewPlatform);
			CreatedPlatforms++;

			UE_LOG(LogTemp, Verbose, TEXT("Created platform %d of type %s at position (%f, %f, %f)"),
				CreatedPlatforms, *UEnum::GetValueAsString(SelectedType),
				WorldPosition.X, WorldPosition.Y, WorldPosition.Z);
		}
		if (PlatformInfo.ResourceType != EResourceType::None)
		{
			SpawnResourceOnPlatform(NewPlatform, PlatformInfo.ResourceType);
		}
	}
	SpawnGoalAtHighestPlatform();
	UE_LOG(LogTemp, Log, TEXT("PlatformPathManager: Created %d platforms out of %d positions"),
		CreatedPlatforms, PlatformCount);
}


void APlatformPathManager::DestroyPlatforms()
{
	for (APlatformComponent* Platform : PlatformComponents)
	{
		if (Platform && IsValid(Platform))
		{
			Platform->Destroy();
		}
	}
	PlatformComponents.Empty();
}

void APlatformPathManager::SetupPlatformAppearance(APlatformComponent* Platform, EPlatformType Type, int MeshIndex)
{
	if (!Platform) return;

	UStaticMesh* SelectedMesh = SelectMeshForType(Type, MeshIndex);

	if (SelectedMesh && Platform->MeshComponent)
	{
		Platform->MeshComponent->SetStaticMesh(SelectedMesh);
	}
}

APlatformComponent* APlatformPathManager::GetPlatformByIndex(int Index) const
{
	if (PlatformComponents.IsValidIndex(Index))
	{
		return PlatformComponents[Index];
	}
	return nullptr;
}

EPlatformType APlatformPathManager::SelectPlatformType(int PlatformIndex, float ZPosition, EPlatformType LastPlatformType)
{
	// Calculate difficulty
	float HeightRatio = FMath::Clamp(ZPosition / SpiralHeight, 0.0f, 1.0f);

	int32 TargetDifficulty = FMath::RoundToInt(HeightRatio * 9.0f) + 1;

	// Todo: difficulty setting
	TArray<EPlatformType> SuitableTypes;
	SuitableTypes = PlatformTypeManager->GetTypesForDifficulty(TargetDifficulty, 2);
	
	if (SuitableTypes.Num() == 0)
	{
		PlatformTypeWeights.GetKeys(SuitableTypes);
	}

	float TotalWeight = 0.0f;
	for (EPlatformType Type : SuitableTypes)
	{
		if (float* Weight = PlatformTypeWeights.Find(Type))
		{
			TotalWeight += *Weight;
		}
	}

	FRandomStream RandomStream(PlatformIndex);
	float RandomValue = RandomStream.FRandRange(0.0f, TotalWeight);

	float AccumulatedWeight = 0.0f;
	for (EPlatformType Type : SuitableTypes)
	{
		if (float* Weight = PlatformTypeWeights.Find(Type))
		{
			AccumulatedWeight += *Weight;
			if (RandomValue <= AccumulatedWeight)
			{
				return Type;
			}
		}
	}

	return EPlatformType::Standard;
}


UStaticMesh* APlatformPathManager::SelectMeshForType(EPlatformType Type, int Seed)
{
	if (FPlatformMeshArray* MeshArray = PlatformMeshesByType.Find(Type))
	{
		if (MeshArray->Meshes.Num() > 0)
		{
			FRandomStream RandomStream(Seed);
			int32 MeshIndex = RandomStream.RandRange(0, MeshArray->Meshes.Num() - 1);
			return MeshArray->Meshes[MeshIndex];
		}
	}

	for (auto& Pair : PlatformMeshesByType)
	{
		if (Pair.Value.Meshes.Num() > 0)
		{
			return Pair.Value.Meshes[0];
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No meshes found for platform type %s"), *UEnum::GetValueAsString(Type));
	return nullptr;
}

void APlatformPathManager::GeneratePlatformSpiralPositions()
{
	PlacedPlatforms.Empty();
	PlatformCount = 0;

	if (SpiralRadius <= 0.0f || SpiralHeight <= 0.0f || SpiralTurns <= 0.0f || PlatformsPerTurn <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid spiral parameters, skipping spiral generation"));
		return;
	}

	int32 TotalPlatforms = FMath::RoundToInt(SpiralTurns * PlatformsPerTurn);
	float VerticalStep = SpiralHeight / FMath::Max(1, TotalPlatforms - 1);
	float AngleStep = (2.0f * PI * SpiralTurns) / FMath::Max(1, TotalPlatforms - 1);
	EPlatformType LastPlatformType = EPlatformType::Standard;

	// Radius expansion factor - adjust this value to control how much the radius grows
	float RadiusGrowthFactor = 1.5f; // Radius will be 1.5x larger at the top compared to bottom

	// Generate spiral positions
	for (int32 i = 0; i < TotalPlatforms; i++)
	{
		float CurrentAngle = i * AngleStep;
		float CurrentHeight = i * VerticalStep;

		FRandomStream SpiralStream(RandomSeed + i);

		// Calculate height-based radius multiplier (0.0 at bottom, 1.0 at top)
		float HeightRatio = CurrentHeight / SpiralHeight;
		float RadiusMultiplier = 1.0f + (RadiusGrowthFactor - 1.0f) * HeightRatio;

		float RadiusVariation = SpiralRadius * SpiralStream.FRandRange(-0.1f, 0.1f);
		float CurrentRadius = (SpiralRadius * RadiusMultiplier) + RadiusVariation;

		float AngleVariation = SpiralStream.FRandRange(-5.0f, 5.0f) * PI / 180.0f;
		CurrentAngle += AngleVariation;

		FVector SpiralPosition;
		SpiralPosition.X = CurrentRadius * FMath::Cos(CurrentAngle);
		SpiralPosition.Y = CurrentRadius * FMath::Sin(CurrentAngle);
		SpiralPosition.Z = CurrentHeight;

		float HeightVariation = VerticalStep * SpiralStream.FRandRange(-0.05f, 0.05f);
		SpiralPosition.Z += HeightVariation;

		EPlatformType SelectedType = SelectPlatformType(i * i + 2, SpiralPosition.Z, LastPlatformType);
		UStaticMesh* SelectedMesh = SelectMeshForType(SelectedType, i);

		if (!SelectedMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("No mesh found for platform %d, skipping"), i);
			continue;
		}
		EResourceType SelectedResourceType = SelectResourceType(i, SpiralPosition.Z);
		FPlacedPlatformInfo NewPlatform(SelectedType, SelectedMesh, SpiralPosition);
		NewPlatform.ResourceType = SelectedResourceType;
		PlacedPlatforms.Add(NewPlatform);
		PlatformCount++;
	}

	PlacedPlatforms[TotalPlatforms - 1].ResourceType = EResourceType::None;
}

EResourceType APlatformPathManager::SelectResourceType(int32 PlatformIndex, float ZPosition)
{
	float TotalWeight = 0.0f;
	for (auto& Pair : ResourceTypeWeights)
		TotalWeight += Pair.Value;

	FRandomStream RandomStream(PlatformIndex * 7);
	float RandomValue = RandomStream.FRandRange(0.0f, TotalWeight);

	float AccumulatedWeight = 0.0f;
	for (auto& Pair : ResourceTypeWeights)
	{
		AccumulatedWeight += Pair.Value;
		if (RandomValue <= AccumulatedWeight)
			return Pair.Key;
	}
	return EResourceType::None;
}

void APlatformPathManager::SpawnResourceOnPlatform(APlatformComponent* Platform, EResourceType ResourceType)
{
	if (!Platform || !ResourcePickupClass || ResourceType == EResourceType::None)
		return;


	FVector SpawnLocation = Platform->GetActorLocation();
	SpawnLocation.Z += ResourceOffsetHeight;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	AResourcePickup* Resource = GetWorld()->SpawnActor<AResourcePickup>(
		ResourcePickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (Resource)
	{
		Resource->AttachToActor(Platform, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void APlatformPathManager::SpawnGoalAtHighestPlatform()
{
	if (!GoalClass || PlatformComponents.Num() == 0)
		return;

	// Find highest platform (existing code unchanged)
	APlatformComponent* HighestPlatform = nullptr;
	float MaxHeight = -FLT_MAX;

	for (APlatformComponent* Platform : PlatformComponents)
	{
		if (Platform->GetActorLocation().Z > MaxHeight)
		{
			MaxHeight = Platform->GetActorLocation().Z;
			HighestPlatform = Platform;
		}
	}

	if (HighestPlatform)
	{
		FVector GoalLocation = HighestPlatform->GetActorLocation();
		GoalLocation.Z += ResourceOffsetHeight;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		ALevelGoal* Goal = GetWorld()->SpawnActor<ALevelGoal>(GoalClass, GoalLocation, FRotator::ZeroRotator, SpawnParams);

		if (Goal)
		{
			Goal->AttachToActor(HighestPlatform, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
}