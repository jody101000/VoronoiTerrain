// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformPathManager.h"
#include "MovingPlatformComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "FortuneAlgorithm/FortuneAlgorithm.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"

APlatformPathManager::APlatformPathManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	PlatformTypeManager = CreateDefaultSubobject<UPlatformTypeManager>(TEXT("PlatformTypeManager"));

	GapSize = FGapSize(100.0f, 200.0f, 100.0f, 200.0f);

	PlatformTypeWeights.Add(EPlatformType::Standard, 0.4f);
	PlatformTypeWeights.Add(EPlatformType::Bounce, 0.2f);
	PlatformTypeWeights.Add(EPlatformType::Rotating, 0.15f);
	PlatformTypeWeights.Add(EPlatformType::Slippery, 0.15f);
	PlatformTypeWeights.Add(EPlatformType::Moving, 0.1f);
}

void APlatformPathManager::BeginPlay()
{
	Super::BeginPlay();

	GeneratePathNet();
	GeneratePlatformPositions();
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

	GeneratePathNet();
	GeneratePlatformPositions();

	if (ShowDebugEdges)
	{
		for (int i = 0; i < VoronoiEdges.Num(); i++)
		{
			const auto& Edge = VoronoiEdges[i];
			FVector Vertex1 = VoronoiVertices[Edge.Get<0>()];
			FVector Vertex2 = VoronoiVertices[Edge.Get<1>()];
			DrawDebugLine(GetWorld(), Vertex1 + GetActorLocation(), Vertex2 + GetActorLocation(), FColor::MakeRandomSeededColor(i), true, -1, 0, 5);
		}
	}

	if (ShowDebugCircles)
	{
		for (const auto& Pos : PlatformPositions)
		{
			OrderVerticesByHeight();

			for (int i = 0; i < PlatformPositions.Num(); i++)
			{
				EPlatformType Type = SelectPlatformType(i, PlatformPositions[i].Z);
				FColor DebugColor = FColor::White;

				switch (Type)
				{
				case EPlatformType::Standard: DebugColor = FColor::White; break;
				case EPlatformType::Bounce: DebugColor = FColor::Green; break;
				case EPlatformType::Rotating: DebugColor = FColor::Blue; break;
				case EPlatformType::Slippery: DebugColor = FColor::Cyan; break;
				case EPlatformType::Moving: DebugColor = FColor::Yellow; break;
				}

				DrawDebugCircle(GetWorld(), PlatformPositions[i] + GetActorLocation(),
					PlatformSize, 24, DebugColor, true, -1, 0, 2,
					FVector(0, 1, 0), FVector(1, 0, 0), false);
			}
		}
	}
}


void APlatformPathManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// UE_LOG(LogTemp, Warning, TEXT("Update Rotation"));

}

// ToDo: 设置某种mesh特性
void APlatformPathManager::CreatePlatforms()
{
	DestroyPlatforms();

	OrderVerticesByHeight();

	struct FPlacedPlatformInfo
	{
		FVector Position;
		float Size;
		UStaticMesh* Mesh;
	};
	TArray<FPlacedPlatformInfo> PlacedPlatforms;

	int CreatedPlatforms = 0;

	// Process platforms from bottom to top
	for (int i = 0; i < PlatformCount; i++)
	{
		if (!PlatformPositions.IsValidIndex(i))
		{
			continue;
		}

		FVector LocalPosition = PlatformPositions[i];
		FVector WorldPosition = GetActorLocation() + LocalPosition;

		EPlatformType SelectedType = SelectPlatformType(i, LocalPosition.Z);

		// Select mesh for this type
		UStaticMesh* SelectedMesh = SelectMeshForType(SelectedType, i);
		if (!SelectedMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("No mesh available for platform type %s"),
				*UEnum::GetValueAsString(SelectedType));
			continue;
		}


		FVector MeshSize = SelectedMesh->GetBounds().GetBox().GetSize();
		float MaxSize = FMath::Max3(MeshSize.X, MeshSize.Y, MeshSize.Z);
		float Scale = PlatformSize / MaxSize * 2.0f;

		bool bHasCollision = false;
		for (const FPlacedPlatformInfo& PlacedInfo : PlacedPlatforms)
		{
			float MinSpacing = CalculateMinimumSpacing(SelectedMesh, PlacedInfo.Mesh, Scale);
			float Distance = FVector::Dist(WorldPosition, PlacedInfo.Position);

			if (Distance < MinSpacing)
			{
				bHasCollision = true;

				FVector Direction = (WorldPosition - PlacedInfo.Position).GetSafeNormal();
				if (!Direction.IsNearlyZero())
				{
					FVector AdjustedPosition = PlacedInfo.Position + Direction * MinSpacing * 1.1f;

					if (FVector::Dist(AdjustedPosition, LocalPosition + GetActorLocation()) < PlatformSize * 0.5f)
					{
						WorldPosition = AdjustedPosition;
						bHasCollision = false;
					}
				}

				if (bHasCollision)
				{
					break;
				}
			}
		}

		if (bHasCollision)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Skipping platform %d due to collision"), i);
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Name = *FString::Printf(TEXT("Platform_%d_%s"), CreatedPlatforms,
			*UEnum::GetValueAsString(SelectedType));

		APlatformComponent* NewPlatform = GetWorld()->SpawnActor<APlatformComponent>(
			APlatformComponent::StaticClass(),
			WorldPosition,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (NewPlatform)
		{
			FRandomStream RandomStream(i);
			FRotator InitialRotation;

			if (SelectedType == EPlatformType::Moving || SelectedType == EPlatformType::Bounce)
			{
				InitialRotation = FRotator(
					RandomStream.FRandRange(-MaxRotationAngle * 0.3f, MaxRotationAngle * 0.3f),
					RandomStream.FRandRange(-180.0f, 180.0f),
					RandomStream.FRandRange(-MaxRotationAngle * 0.3f, MaxRotationAngle * 0.3f)
				);
			}
			else
			{
				InitialRotation = FRotator(
					RandomStream.FRandRange(-MaxRotationAngle, MaxRotationAngle),
					RandomStream.FRandRange(-180.0f, 180.0f),
					RandomStream.FRandRange(-MaxRotationAngle, MaxRotationAngle)
				);
			}

			// Initialize platform
			NewPlatform->InitializePlatform(SelectedType, SelectedMesh, CreatedPlatforms,
				WorldPosition, InitialRotation, Scale);
			NewPlatform->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

			PlatformComponents.Add(NewPlatform);

			FPlacedPlatformInfo PlacedInfo;
			PlacedInfo.Position = WorldPosition;
			PlacedInfo.Size = Scale * MaxSize;
			PlacedInfo.Mesh = SelectedMesh;
			PlacedPlatforms.Add(PlacedInfo);

			CreatedPlatforms++;

			UE_LOG(LogTemp, Verbose, TEXT("Created platform %d of type %s at position (%f, %f, %f)"),
				CreatedPlatforms, *UEnum::GetValueAsString(SelectedType),
				WorldPosition.X, WorldPosition.Y, WorldPosition.Z);
		}
	}

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

	// for (AInteractivePlatform* Platform : InteractivePlatforms)
	// {
	// 	if (Platform && IsValid(Platform))
	// 	{
	// 		Platform->Destroy();
	// 	}
	// }
	// InteractivePlatforms.Empty();
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


// void APlatformPathManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
// {
// 	Super::PostEditChangeProperty(PropertyChangedEvent);
//
// 	DestroyPlatforms();
//
// 	if (PropertyChangedEvent.Property)
// 	{
// 		FString PropertyName = PropertyChangedEvent.Property->GetName();
//
// 		if (PropertyName == TEXT("GapSize") || 
// 			PropertyName == TEXT("PlatformSize") ||
// 			PropertyName == TEXT("SectionSize") ||
// 			PropertyName == TEXT("RandomSeed") ||
// 			PropertyName == TEXT("SiteCount"))
// 		{
// 			CreatePlatforms();
// 		}
// 	}
// }

void APlatformPathManager::GenerateRandomPoints()
{
	VoronoiSitePoints2D.clear();
	
	FVector BoundsCenter = SectionSize.GetCenter();
	FVector BoundsExtent = SectionSize.GetExtent();
	const FRandomStream RandomStream(RandomSeed);
	for (int i = 0; i < SiteCount; i++)
	{
		FVector Point = UKismetMathLibrary::RandomPointInBoundingBoxFromStream(RandomStream, BoundsCenter, BoundsExtent);
		VoronoiSitePoints2D.push_back({Point.X, Point.Z});
	}
}

TArray<TTuple<int, int>> APlatformPathManager::ConvertEdgesToIndices(
	const TArray<FVector>& Vertices,
	const TArray<TTuple<FVector, FVector>>& PositionEdges) const
{
	TArray<TTuple<int, int>> IndexEdges;
	IndexEdges.Reserve(PositionEdges.Num());
    
	for (const auto& Edge : PositionEdges)
	{
		FVector Vertex1 = Edge.Get<0>();
		FVector Vertex2 = Edge.Get<1>();
		
		int32 Index1 = Vertices.IndexOfByPredicate([&](const FVector& V) {
			return V.Equals(Vertex1, KINDA_SMALL_NUMBER);
		});
        
		int32 Index2 = Vertices.IndexOfByPredicate([&](const FVector& V) {
			return V.Equals(Vertex2, KINDA_SMALL_NUMBER);
		});
		
		if (Index1 != INDEX_NONE && Index2 != INDEX_NONE)
		{
			IndexEdges.Add(TTuple<int, int>(Index1, Index2));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Edge vertex not found in vertices array"));
		}
	}
	
	IndexEdges.Sort([](const TTuple<int, int>& A, const TTuple<int, int>& B) {
		if (A.Get<0>() == B.Get<0>())
		{
			return A.Get<1>() < B.Get<1>();
		}
		return A.Get<0>() < B.Get<0>();
	});
	
	return IndexEdges;
}

void APlatformPathManager::GenerateVoronoiEdges()
{
	VoronoiVertices.Empty();
	VoronoiEdges.Empty();
	
	TArray<TTuple<FVector, FVector>> VoronoiPositionEdges;
	
	FortuneAlgorithm algorithm(VoronoiSitePoints2D);
	algorithm.construct();
	const double MaxX = SectionSize.SizeX;
	const double MaxZ = SectionSize.SizeZ;
	algorithm.bound(Box{-0.05f, -0.05f, MaxX+0.05f, MaxZ+0.05f});
	VoronoiDiagram Diagram = algorithm.getDiagram();
	Diagram.intersect(Box{0.0f, 0.0f, MaxX, MaxZ});

	std::list<VoronoiDiagram::HalfEdge> Edges = Diagram.getHalfEdges();
	while (!Edges.empty())
	{
		VoronoiDiagram::HalfEdge& HalfEdge = Edges.front();
		FVector Start = FVector(HalfEdge.origin->point.x, 0.0f, HalfEdge.origin->point.y);
		FVector End = FVector(HalfEdge.destination->point.x, 0.0f, HalfEdge.destination->point.y);
		if (!HalfEdge.twin && (Start.Z - End.Z) < KINDA_SMALL_NUMBER) // Horizontal bounding edges
		{
			auto PositionEdge = TTuple<FVector, FVector>(Start, End);
			VoronoiPositionEdges.Add(PositionEdge);
		}
		else if (HalfEdge.twin) // Internal edges
		{
			auto PositionEdge = TTuple<FVector, FVector>(Start, End);
			VoronoiPositionEdges.Add(PositionEdge);
			Edges.pop_front(); // Pop duplicated edge (twin)
		}
		Edges.pop_front();
		
	}
	std::list<VoronoiDiagram::Vertex> Vertices = Diagram.getVertices();
	for (auto& Vertex : Vertices)
	{
		VoronoiVertices.Add({Vertex.point.x, 0, Vertex.point.y});
	}

	VoronoiEdges = ConvertEdgesToIndices(VoronoiVertices, VoronoiPositionEdges);
	VoronoiPositionEdges.Empty();
}

// Compute Y shift needed to get the target angle
float ComputeYShift(const FVector& CurrVector, float TargetAngle, float Scale)
{
	// Cone x^2 + y^2 = z^2 * tan^2(theta)
	float Z = CurrVector.Z;
	float X = CurrVector.X;
	float YSquared = FMath::Square(Z) * FMath::Square(FMath::Tan(TargetAngle)) - FMath::Square(X);
	YSquared = FMath::Sqrt(YSquared);
	
	return YSquared * Scale;
}


void APlatformPathManager::InclinedVoronoiEdges()
{
	float MinAngle = FMath::DegreesToRadians(MinAngleDegree);
	float MaxAngle = FMath::DegreesToRadians(MaxAngleDegree);

	for (int i = 0; i < VoronoiEdges.Num(); i++)
	{
		const auto& Edge = VoronoiEdges[i];
		int v1 = Edge.Get<0>();
		int v2 = Edge.Get<1>();
		// Make sure start vertex is the one with lower Z
		if (VoronoiVertices[v1].Z > VoronoiVertices[v2].Z)
		{
			v1 = Edge.Get<1>();
			v2 = Edge.Get<0>();
		}
		
		FVector& StartVertex = VoronoiVertices[v1];
		FVector& EndVertex = VoronoiVertices[v2];
		
		FVector EdgeVector = EndVertex - StartVertex;
		float Scale = EdgeVector.Length();
		EdgeVector.Normalize();
		
		float InclineAngleRad = FMath::Abs(FMath::Acos(FVector::DotProduct(EdgeVector, FVector::UnitZ())));

		const FRandomStream RandomStream(i);
		int DirectionY = UKismetMathLibrary::RandomBoolFromStream(i) ? 1 : -1;
		if (InclineAngleRad < MinAngle) // Too steep, more y incline
		{
			float TargetAngle = UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStream, MinAngle, MaxAngle);
			float YShift = ComputeYShift(EdgeVector, TargetAngle, Scale) * DirectionY;
			EndVertex.Y = YShift;
			// UE_LOG(LogTemp, Warning, TEXT("Shift Vertex %d Y from "), switched ? v1 : v2);
		}
	}
}

void APlatformPathManager::GeneratePathNet()
{
	GenerateRandomPoints();
	GenerateVoronoiEdges();
	UE_LOG(LogTemp, Warning, TEXT("Created %d Vertices"), VoronoiVertices.Num());
	InclinedVoronoiEdges();
}

FVector FindPointOnArc(FVector StartPos, FVector EndPos, bool bUsePositiveSide, float T)
{
	FVector Midpoint = (StartPos + EndPos) * 0.5f;
	
	FVector StartToEnd = EndPos - StartPos;
	float StartEndDistance = StartToEnd.Size();
	
	FVector Perpendicular = FVector(-StartToEnd.Y, StartToEnd.X, 0.0f);
	Perpendicular = Perpendicular.GetSafeNormal();
	
	float OffsetDistance = FMath::Sqrt(3.0f) * StartEndDistance * 0.5f;

	if (!bUsePositiveSide)
	{
		OffsetDistance = -OffsetDistance;
	}
	
	FVector CenterPos = Midpoint + (Perpendicular * OffsetDistance);
	
	FVector StartVec = StartPos - CenterPos;
	FVector EndVec = EndPos - CenterPos;
	float Radius = StartVec.Size();
    
	float StartAngle = FMath::Atan2(StartVec.Y, StartVec.X);
	float EndAngle = FMath::Atan2(EndVec.Y, EndVec.X);
	
	float AngleDiff = EndAngle - StartAngle;
	if (AngleDiff > PI)
	{
		AngleDiff -= 2.0f * PI;
	}
	else if (AngleDiff < -PI)
	{
		AngleDiff += 2.0f * PI;
	}
	
	float CurrentAngle = StartAngle + (AngleDiff * T);
	
	FVector Result;
	Result.X = CenterPos.X + Radius * FMath::Cos(CurrentAngle);
	Result.Y = CenterPos.Y + Radius * FMath::Sin(CurrentAngle);
	Result.Z = FMath::Lerp(StartPos.Z, EndPos.Z, T); // Linear interpolation for Z
    
	return Result;
}

// Generate positions from edges
// ToDo: platform距离根据
// 1. 当前mesh的特性：旋转、倾斜、光滑程度、弹性、AABB/OBB
// 2. 角色跳跃能力
void APlatformPathManager::GeneratePlatformPositions()
{
	PlatformPositions.Empty();
	PlatformCount = 0;

	for (const auto& Vertex : VoronoiVertices)
	{
		PlatformPositions.Add(Vertex);
		PlatformCount++;
	}

	for (const auto& Edge : VoronoiEdges)
	{
		int v1 = Edge.Get<0>();
		int v2 = Edge.Get<1>();
		const FVector& StartVertex = VoronoiVertices[v1];
		const FVector& EndVertex = VoronoiVertices[v2];
		float EdgeLength = FVector::Dist(StartVertex, EndVertex);

		// Calculate platform count based on edge length and gap settings
		const FRandomStream RandomStream(v1 * 100 + v2);
		float XYGap = RandomStream.FRandRange(GapSize.MinXY, GapSize.MaxXY) + PlatformSize;
		float ZGap = RandomStream.FRandRange(GapSize.MinZ, GapSize.MaxZ) + PlatformSize;

		float EffectiveGap = FMath::Min(XYGap, ZGap);
		int PlatformNum = FMath::Max(2, static_cast<int>(EdgeLength / EffectiveGap));

		for (int i = 1; i < PlatformNum; i++)
		{
			float Ratio = static_cast<float>(i) / PlatformNum;

			// Use arc or linear interpolation
			bool bUseArc = RandomStream.FRandRange(0.0f, 1.0f) > 0.5f;
			FVector Position;

			if (bUseArc)
			{
				Position = FindPointOnArc(StartVertex, EndVertex, true, Ratio);
			}
			else
			{
				Position = FMath::Lerp(StartVertex, EndVertex, Ratio);
			}

			PlatformPositions.Add(Position);
			PlatformCount++;
		}
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

void APlatformPathManager::OrderVerticesByHeight()
{
	SortedVertexIndices.Empty();

	for (int32 i = 0; i < VoronoiVertices.Num(); i++)
	{
		SortedVertexIndices.Add(i);
	}

	SortedVertexIndices.Sort([this](const int32& A, const int32& B)
		{
			return VoronoiVertices[A].Z < VoronoiVertices[B].Z;
		});
}

EPlatformType APlatformPathManager::SelectPlatformType(int PlatformIndex, float ZPosition)
{
	// Calculate difficulty
	float MaxZ = VoronoiVertices.Num() > 0 ?
		VoronoiVertices[SortedVertexIndices.Last()].Z : SectionSize.SizeZ;
	float HeightRatio = FMath::Clamp(ZPosition / MaxZ, 0.0f, 1.0f);

	int32 TargetDifficulty = FMath::RoundToInt(HeightRatio * 9.0f) + 1;

	TArray<EPlatformType> SuitableTypes = PlatformTypeManager->GetTypesForDifficulty(TargetDifficulty, 2);

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

bool APlatformPathManager::CheckPlatformCollision(const FVector& Position, UStaticMesh* Mesh, float Scale)
{
	if (!Mesh) return false;

	FVector MeshSize = Mesh->GetBounds().GetBox().GetSize() * Scale;
	FBox TestBox = FBox::BuildAABB(Position, MeshSize / 2.0f);

	for (const APlatformComponent* ExistingPlatform : PlatformComponents)
	{
		if (!ExistingPlatform || !ExistingPlatform->MeshComponent ||
			!ExistingPlatform->MeshComponent->GetStaticMesh())
		{
			continue;
		}

		FVector ExistingSize = ExistingPlatform->MeshComponent->GetStaticMesh()->GetBounds().GetBox().GetSize() *
			ExistingPlatform->GetActorScale3D().X;
		FBox ExistingBox = FBox::BuildAABB(ExistingPlatform->GetActorLocation(), ExistingSize / 2.0f);

		FBox ExpandedBox = ExistingBox.ExpandBy(PlatformSize * 0.1f); // 10% safety margin

		if (TestBox.Intersect(ExpandedBox))
		{
			return true; // Collision detected
		}
	}

	return false;
}

float APlatformPathManager::CalculateMinimumSpacing(UStaticMesh* Mesh1, UStaticMesh* Mesh2, float Scale)
{
	if (!Mesh1 || !Mesh2)
	{
		return PlatformSize * 2.0f; // Default spacing
	}

	FVector Size1 = Mesh1->GetBounds().GetBox().GetSize() * Scale;
	FVector Size2 = Mesh2->GetBounds().GetBox().GetSize() * Scale;

	// maximum dimension
	float MaxDim1 = FMath::Max3(Size1.X, Size1.Y, Size1.Z);
	float MaxDim2 = FMath::Max3(Size2.X, Size2.Y, Size2.Z);
	float MinSpacing = (MaxDim1 + MaxDim2) * 0.5f + PlatformSize * 0.2f;

	float ConfiguredGap = FMath::Min(GapSize.MinXY, GapSize.MinZ);

	return FMath::Max(MinSpacing, ConfiguredGap);
}