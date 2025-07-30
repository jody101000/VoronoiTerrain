// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformPathManager.h"
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

	PlatformMeshesByType.Add(EPlatformType::Standard, FPlatformMeshArray());
	PlatformMeshesByType.Add(EPlatformType::Bounce, FPlatformMeshArray());
	PlatformMeshesByType.Add(EPlatformType::Rotating, FPlatformMeshArray());
	PlatformMeshesByType.Add(EPlatformType::Slippery, FPlatformMeshArray());
	PlatformMeshesByType.Add(EPlatformType::Moving, FPlatformMeshArray());
	
}

void APlatformPathManager::BeginPlay()
{
	Super::BeginPlay();

	// GeneratePathNet();
	GenerateFlatPathNet();
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

	// GeneratePathNet();
	GenerateFlatPathNet();
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
		for (const auto& PlatformInfo : PlacedPlatforms)
		{
			EPlatformType Type = PlatformInfo.Type;
			FColor DebugColor = FColor::White;

			switch (Type)
			{
			case EPlatformType::Standard: DebugColor = FColor::White; break;
			case EPlatformType::Bounce: DebugColor = FColor::Green; break;
			case EPlatformType::Rotating: DebugColor = FColor::Blue; break;
			case EPlatformType::Slippery: DebugColor = FColor::Cyan; break;
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

// ToDo: 设置某种mesh特性
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

		// ToDo: how to setup collision bounds on spawning so actor can detect collision?
		APlatformComponent* NewPlatform = GetWorld()->SpawnActor<APlatformComponent>(
			APlatformComponent::StaticClass(),
			WorldPosition,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (NewPlatform)
		{
			NewPlatform->InitializePlatform(SelectedType, SelectedMesh, CreatedPlatforms,
				WorldPosition, FRotator::ZeroRotator, Scale);
			NewPlatform->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

			PlatformComponents.Add(NewPlatform);
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


void APlatformPathManager::OrderVerticesByHeight()
{
	VoronoiVertices.Sort([this](const FVector& A, const FVector& B)
		{
			return A.Z < B.Z;
		});
}

void APlatformPathManager::OrderEdgesByHeight()
{
	VoronoiEdges.Sort([this](const TTuple<int, int>& A, const TTuple<int, int>& B)
	{
		if (A.Get<0>() != B.Get<0>())
		{
			return A.Get<0>() < B.Get<0>();
		}
		return A.Get<1>() < B.Get<1>();
	});
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
	
	
	OrderVerticesByHeight();
	VoronoiEdges = ConvertEdgesToIndices(VoronoiVertices, VoronoiPositionEdges);
	OrderEdgesByHeight();
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

	for (int i = VoronoiEdges.Num() - 1; i >= 0; i--)
	{
		auto& Edge = VoronoiEdges[i];
		int v1 = Edge.Get<0>();
		int v2 = Edge.Get<1>();
		FVector& LowVertex = VoronoiVertices[v1];
		FVector& HighVertex = VoronoiVertices[v2];
		float Dist = FVector::Dist(LowVertex, HighVertex);
		if (Dist <= PlatformSize * 2)
		{
			VoronoiEdges.RemoveAt(i);
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

void APlatformPathManager::GenerateFlatPathNet()
{
	// Generate Random Points
	VoronoiSitePoints2D.clear();
	
	FVector BoundsCenter = SectionSize.GetCenter();
	FVector BoundsExtent = SectionSize.GetExtent();
	const FRandomStream RandomStream(RandomSeed);
	for (int i = 0; i < SiteCount; i++)
	{
		FVector Point = UKismetMathLibrary::RandomPointInBoundingBoxFromStream(RandomStream, BoundsCenter, BoundsExtent);
		VoronoiSitePoints2D.push_back({Point.X, Point.Z});
	}

	// Generate Vertices and Edges
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
		FVector Start = FVector(HalfEdge.origin->point.x, HalfEdge.origin->point.y, 0.0f);
		FVector End = FVector(HalfEdge.destination->point.x, HalfEdge.destination->point.y, 0.0f);
		if (!HalfEdge.twin && (Start.Y - End.Y) < KINDA_SMALL_NUMBER) // bounding edges
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
		VoronoiVertices.Add({Vertex.point.x, Vertex.point.y, 0});
	}
	
	
	// OrderVerticesByY
	VoronoiVertices.Sort([this](const FVector& A, const FVector& B)
	{
		return A.Y < B.Y;
	});
	VoronoiEdges = ConvertEdgesToIndices(VoronoiVertices, VoronoiPositionEdges);
	
	OrderEdgesByHeight();
	
	VoronoiPositionEdges.Empty();

	for (int i = VoronoiEdges.Num() - 1; i >= 0; i--)
	{
		auto& Edge = VoronoiEdges[i];
		int v1 = Edge.Get<0>();
		int v2 = Edge.Get<1>();
		FVector& LowVertex = VoronoiVertices[v1];
		FVector& HighVertex = VoronoiVertices[v2];
		float Dist = FVector::Dist(LowVertex, HighVertex);
		if (Dist <= PlatformSize * 2)
		{
			VoronoiEdges.RemoveAt(i);
		}
	}

	
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
// 正上方不能有platform
void APlatformPathManager::GeneratePlatformPositions()
{
	PlacedPlatforms.Empty();

	// Platforms at all Vertices
	for (int i = 0; i < VoronoiVertices.Num(); i++)
	{
		FVector& Vertex = VoronoiVertices[i];
		EPlatformType SelectedType = SelectPlatformType(i, Vertex.Z);
		UStaticMesh* SelectedMesh = SelectMeshForType(SelectedType, i);

		//ToDo: Select Type for Short Edge Without midpoint
		FPlacedPlatformInfo NewPlatform(SelectedType, SelectedMesh, Vertex);
		PlacedPlatforms.Add(NewPlatform);
		PlatformCount++;
	}

	// ToDo: Fan-shaped next point range
	for (const auto& Edge : VoronoiEdges)
	{
		int v1 = Edge.Get<0>();
		int v2 = Edge.Get<1>();
		const FVector& Start = VoronoiVertices[v1];
		const FVector& End = VoronoiVertices[v2];
		float XYLength = FVector::Dist(FVector(Start.X, Start.Y, 0), FVector(End.X, End.Y, 0));
		// int PlatformNumOnEdge = (Length - 2 * PlatformSize) / (PlatformSize * 2);
		
		const FRandomStream FirstStream(v1);
		float XYstep = 2 * PlatformSize + UKismetMathLibrary::RandomFloatInRangeFromStream(FirstStream, GapSize.MinXY, GapSize.MaxXY);
		float RelativeXY = XYstep;
		while (RelativeXY < XYLength - 2 * PlatformSize)
		{
			FVector Position = FMath::Lerp(Start, End, RelativeXY / XYLength);
			
			int index = PlacedPlatforms.Num();
			EPlatformType SelectedType = SelectPlatformType(index, Position.X);
			UStaticMesh* SelectedMesh = SelectMeshForType(SelectedType, index);

			// ToDo: should also depends on bouncing pad
			FPlacedPlatformInfo NewPlatform(SelectedType, SelectedMesh, Position);
			PlacedPlatforms.Add(NewPlatform);
			PlatformCount++;
			
			const FRandomStream Stream(RelativeXY / XYLength);
			XYstep = 2 * PlatformSize + UKismetMathLibrary::RandomFloatInRangeFromStream(Stream, GapSize.MinXY, GapSize.MaxXY);
			RelativeXY += XYstep;
		}
	}
	
	CheckPlatformInfoCollisions();
}

APlatformComponent* APlatformPathManager::GetPlatformByIndex(int Index) const
{
	if (PlatformComponents.IsValidIndex(Index))
	{
		return PlatformComponents[Index];
	}
	return nullptr;
}

EPlatformType APlatformPathManager::SelectPlatformType(int PlatformIndex, float ZPosition)
{
	// Calculate difficulty
	float HeightRatio = FMath::Clamp(ZPosition / SectionSize.SizeZ, 0.0f, 1.0f);

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

void APlatformPathManager::CheckPlatformInfoCollisions()
{
	
	int i = 0;
	for (FPlacedPlatformInfo& ThisInfo : PlacedPlatforms)
	{
		// double check collision
		bool bHasCollision = false;
		// for (const FPlacedPlatformInfo& CompareInfo : PlacedPlatforms)
		// {
		// 	FMatrix Transform1 = FMatrix::Identity;
		// 	Transform1 = Transform1.ApplyScale(ThisInfo.Scale);
		// 	FMatrix Transform2 = FMatrix::Identity;
		// 	Transform2 = Transform2.ApplyScale(CompareInfo.Scale);
		// 	FBox Box1 = ThisInfo.Mesh->GetBoundingBox();
		// 	FBox Box2 = CompareInfo.Mesh->GetBoundingBox();
		// 	Box1 = Box1.ShiftBy(ThisInfo.Position);
		// 	Box2 = Box2.ShiftBy(CompareInfo.Position);
		// 	
		// 	if (Box1.Intersect(Box2))
		// 	{
		// 		bHasCollision = true;
		// 	
		// 		FVector Direction = (ThisInfo.Position - CompareInfo.Position).GetSafeNormal();
		// 		if (!Direction.IsNearlyZero())
		// 		{
		// 			FVector AdjustedPosition = ThisInfo.Position + Direction * GapSize.MinXY;
		// 		}
		// 	
		// 		if (bHasCollision)
		// 		{
		// 			break;
		// 		}
		// 	}
		// }

		if (bHasCollision)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Skipping platform %d due to collision"), i);
			continue;
		}
		i++;
	}
}
