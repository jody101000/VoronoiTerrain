#include "PlatformSystem/PlatformPathManager.h"

#include "PlatformSystem/PlatformTypeManager.h"
#include "Actors/LevelGoal.h"
#include "Actors/ResourcePickup.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

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
}

void APlatformPathManager::BeginPlay()
{
	Super::BeginPlay();

	GenerateLinearPlatformPositions();
	CreatePlatforms();
}

void APlatformPathManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	GenerateLinearPlatformPositions();

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

void APlatformPathManager::GenerateLinearPlatformPositions()
{
	PlacedPlatforms.Empty();

	// Calculate line direction and total length
	FVector LineDirection = (EndPosition - StartPosition).GetSafeNormal();
	float TotalLineLength = FVector::Dist(StartPosition, EndPosition);

	if (LineDirection.IsNearlyZero() || TotalLineLength <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid line segment, skipping generation"));
		return;
	}

	// Calculate perpendicular direction for orthogonal shifts
	FVector BasePerpendicular = FVector::CrossProduct(LineDirection, FVector::UpVector).GetSafeNormal();

	float CurrentDistance = 0.0f;
	float StandardWeight = *PlatformTypeWeights.Find(EPlatformType::Standard);
	float MovingWeight = *PlatformTypeWeights.Find(EPlatformType::Moving);

	for (int32 i = 0; i < PlatformCount; i++)
	{
		FRandomStream RandomStream(RandomSeed + i);
		float RandomAngle = UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStream, -60, 60);
		FVector PerpendicularDirection = BasePerpendicular.RotateAngleAxis(RandomAngle, LineDirection);

		// Calculate position along the line
		FVector BasePosition;
		if (i == 0)
		{
			BasePosition = StartPosition;
			CurrentDistance = 0.0f;
		}
		else
		{
			// Random distance for intermediate platforms
			float RandomDistance = RandomStream.FRandRange(PlatformDistanceRange.X, PlatformDistanceRange.Y);
			CurrentDistance += RandomDistance;
			BasePosition = StartPosition + (LineDirection * CurrentDistance);
		}

		// Apply orthogonal shift
		float OrthogonalShift = RandomStream.FRandRange(OrthogonalShiftRange.X, OrthogonalShiftRange.Y);
		FVector FinalPosition = BasePosition;
		if (i > 0)
		{
			FinalPosition  += PerpendicularDirection * OrthogonalShift;
		}

		// Select platform type
		EPlatformType SelectedType = SelectPlatformType(StandardWeight, MovingWeight);

		// Select mesh for this platform type
		UStaticMesh* SelectedMesh = SelectMeshForType(SelectedType, RandomSeed + i);
		if (!SelectedMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("No mesh found for platform %d, skipping"), i);
			continue;
		}

		// Select resource type (no resource on last platform)
		EResourceType SelectedResourceType = EResourceType::None;

		// Create platform info
		FPlacedPlatformInfo NewPlatform(SelectedType, SelectedMesh, FinalPosition);
		NewPlatform.ResourceIndex = -1;
		PlacedPlatforms.Add(NewPlatform);

		UE_LOG(LogTemp, Warning, TEXT("Generated platform %d of type %s at position (%f, %f, %f)"),
			i, *UEnum::GetValueAsString(SelectedType),
			FinalPosition.X, FinalPosition.Y, FinalPosition.Z);
	}

	UE_LOG(LogTemp, Warning, TEXT("Generated %d platforms along line from (%f, %f, %f) to (%f, %f, %f)"),
		PlatformCount, StartPosition.X, StartPosition.Y, StartPosition.Z,
		EndPosition.X, EndPosition.Y, EndPosition.Z);
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

			NewPlatform->PlatformProperties = PlatformTypeManager->GetPlatformTypeProperties(SelectedType);

			NewPlatform->PostInitializePlatform(WorldPosition, FRotator::ZeroRotator, Scale);
			NewPlatform->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

			PlatformComponents.Add(NewPlatform);
			CreatedPlatforms++;

			UE_LOG(LogTemp, Verbose, TEXT("Created platform %d of type %s at position (%f, %f, %f)"),
				CreatedPlatforms, *UEnum::GetValueAsString(SelectedType),
				WorldPosition.X, WorldPosition.Y, WorldPosition.Z);
		}
	}
	SpawnResourcesOnPlatforms();
	SpawnGoalAtHighestPlatform();
	SpawnGoalAtHighestPlatform();
	UE_LOG(LogTemp, Log, TEXT("PlatformPathManager: Created %d platforms out of %d positions"),
		CreatedPlatforms, PlatformCount);
}

void APlatformPathManager::SpawnResourcesOnPlatforms()
{
	if (ResourcePickupClasses.Num() != 3 || PlatformComponents.Num() <= 1)
		return;

	// Create list of available platform indices (excluding last one)
	TArray<int32> AvailablePlatformIndices;
	for (int32 i = 0; i < PlatformComponents.Num() - 1; i++)
	{
		AvailablePlatformIndices.Add(i);
	}

	// Randomly place each resource type on a different platform
	FRandomStream RandomStream(RandomSeed);
	for (int32 ResourceType = 0; ResourceType < 3; ResourceType++)
	{
		if (AvailablePlatformIndices.Num() == 0)
			break;

		// Pick random platform from available ones
		int32 RandomIndex = RandomStream.RandRange(0, AvailablePlatformIndices.Num() - 1);
		int32 PlatformIndex = AvailablePlatformIndices[RandomIndex];
		AvailablePlatformIndices.RemoveAt(RandomIndex);

		// Spawn the resource
		if (PlatformComponents.IsValidIndex(PlatformIndex))
		{
			SpawnResourceOnPlatform(PlatformComponents[PlatformIndex], ResourceType);
		}
	}
}

void APlatformPathManager::SpawnResourceOnPlatform(APlatformComponent* Platform, int32 ResourceTypeIndex)
{
	if (!Platform || !ResourcePickupClasses.IsValidIndex(ResourceTypeIndex))
		return;

	FVector SpawnLocation = Platform->GetActorLocation();
	SpawnLocation.Z += ResourceOffsetHeight;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	AResourcePickup* Resource = GetWorld()->SpawnActor<AResourcePickup>(
		ResourcePickupClasses[ResourceTypeIndex], SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (Resource)
	{
		Resource->AttachToActor(Platform, FAttachmentTransformRules::KeepWorldTransform);
	}
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

EPlatformType APlatformPathManager::SelectPlatformType(float StandardWeight, float MoveWeight)
{
	float TotalWeight = StandardWeight + MoveWeight;

	if (TotalWeight == 0.0f)
	{
		return EPlatformType::Standard;
	}

	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	if (RandomValue <= StandardWeight)
	{
		return EPlatformType::Standard;
	}
	return EPlatformType::Moving;

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


void APlatformPathManager::SpawnGoalAtHighestPlatform()
{
	if (!GoalClass || PlatformComponents.Num() == 0)
		return;

	// Find highest platform (existing code unchanged)
	APlatformComponent* HighestPlatform = nullptr;
	float MaxHeight = -FLT_MAX;

	for (APlatformComponent* Platform : PlatformComponents)
	{
		if (Platform->GetActorLocation().X > MaxHeight)
		{
			MaxHeight = Platform->GetActorLocation().X;
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