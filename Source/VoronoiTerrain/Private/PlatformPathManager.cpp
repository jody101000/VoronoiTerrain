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

	SetupPlatformTypes();

	GapSize = FGapSize(100.0f, 200.0f, 100.0f, 200.0f);
}

void APlatformPathManager::BeginPlay()
{
	Super::BeginPlay();

	GeneratePathNet();
	GeneratePlatformPositions();
	CreatePlatforms();

	for (int i = 0; i < PlatformCount; i++)
	{
		if (i % 5 == 0)
		{
			FRotator NewRotation = FRotator(0,0,0.5);
			PlatformComponents[i]->SetRotationUpdate(NewRotation);
		}
		if (i % 5 == 1)
		{
			FVector Velocity = FVector(0.5,0,0);
			PlatformComponents[i]->SetPositionUpdate(Velocity, 100.0);
		}
	}
}

void APlatformPathManager::OnConstruction(const FTransform& Transform)
{
	FlushPersistentDebugLines(GetWorld());
	GeneratePathNet();
	GeneratePlatformPositions();
	CreatePlatforms();
	
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
			DrawDebugCircle(GetWorld(), Pos + GetActorLocation(), PlatformSize, 24, FColor::Orange, true, -1, 0, 2, FVector(0, 1, 0), FVector(1, 0, 0), false);
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

	for (int i = 0; i < PlatformCount; i++)
	{
		FString ComponentName = FString::Printf(TEXT("PlatformComponent_%d"), i);
		UMovingPlatformComponent* NewPlatform = NewObject<UMovingPlatformComponent>(
			this,
			UMovingPlatformComponent::StaticClass(),
			*ComponentName
		);
		
		const FRandomStream RandomStream(i * i + i);
		int UseMesh = UKismetMathLibrary::RandomIntegerInRangeFromStream(RandomStream, 0, PlatformMesh.Num() - 1);
		
		const FRandomStream RandomStreamRoll(i);
		float Roll = UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStreamRoll, 0, MaxRotationAngle);
		const FRandomStream RandomStreamPitch(2 * i);
		float Pitch = UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStreamPitch, 0, MaxRotationAngle);
		const FRandomStream RandomStreamYaw(3* i);
		float Yaw = UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStreamYaw, 0, MaxRotationAngle);

		if (NewPlatform)
		{
			NewPlatform->RegisterComponent();
			NewPlatform->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
			SetupPlatformAppearance(NewPlatform, UseMesh);

			FVector MeshSize = FVector(1.0f);
			if (PlatformMesh.IsValidIndex(UseMesh) && PlatformMesh[UseMesh])
			{
				MeshSize = PlatformMesh[UseMesh]->GetBounds().GetBox().GetSize();
				// UE_LOG(LogTemp, Log, TEXT("Selected mesh size: (%f, %f, %f)"), MeshSize.X, MeshSize.Y, MeshSize.Z);
			}

			// Set position
			if (PlatformPositions.IsValidIndex(i))
			{
				FVector WorldPosition = GetActorLocation() + PlatformPositions[i];
				// ToDo: scaling for both X and Y axis
				FRotator WorldRotation = FRotator(Pitch, Yaw, Roll);
				float MaxSize = FMath::Max(MeshSize.X, FMath::Max(MeshSize.Y, MeshSize.Z));
				NewPlatform->InitializePlatform(i, WorldPosition, WorldRotation, PlatformSize / MaxSize * 2.0f);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("PlatformComponent_%d's position or radius is not generated correctly"), i);
			}
			
			PlatformComponents.Add(NewPlatform);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("PlatformPathManager: Created %d platforms"), PlatformComponents.Num());
	//
	// if (GetWorld())
	// {
	// 	FVector SpawnLocation = FVector::ZeroVector;
	// 	FRotator SpawnRotation = FRotator::ZeroRotator;
	//
	// 	FActorSpawnParameters SpawnParams;
	// 	SpawnParams.Owner = this;
	// 	SpawnParams.Instigator = GetInstigator();
	//
	// 	AInteractivePlatform* NewActor = GetWorld()->SpawnActor<AInteractivePlatform>(AInteractivePlatform::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
	// 	NewActor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	// 	InteractivePlatforms.Add(NewActor);
	// 	if (NewActor)
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("InteractivePlatform spawned successfully!"));
	// 	}
	// }
}

void APlatformPathManager::DestroyPlatforms()
{
	for (UMovingPlatformComponent* Platform : PlatformComponents)
	{
		if (Platform && IsValid(Platform))
		{
			Platform->DestroyComponent();
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

void APlatformPathManager::SetupPlatformAppearance(UMovingPlatformComponent* Platform, int UseMesh)
{
	if (!Platform) return;
	if (PlatformMesh.IsValidIndex(UseMesh) && PlatformMesh[UseMesh])
	{
		Platform->SetStaticMesh(PlatformMesh[UseMesh]);
	}
	
	if (PlatformMaterial)
	{
		Platform->SetMaterial(0, PlatformMaterial);
	}
}

void APlatformPathManager::SetupPlatformTypes()
{
	PlatformTypeManager = NewObject<UPlatformTypeManager>();
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
	for (const auto& Edge : VoronoiEdges)
	{
		int v1 = Edge.Get<0>();
		int v2 = Edge.Get<1>();
		const FVector& StartVertex = VoronoiVertices[v1];
		const FVector& EndVertex = VoronoiVertices[v2];
		float EdgeLength = FVector::Dist(StartVertex, EndVertex);
		
		// according to density, fin point on arc
		const FRandomStream RandomStream(0);
		float XYGap = UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStream, GapSize.MinXY, GapSize.MaxXY) + PlatformSize;
		float ZGap = UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStream, GapSize.MinZ, GapSize.MaxZ) + PlatformSize;
		int PlatformNum = static_cast<int>(ceil(EdgeLength / FMath::Min(ZGap, XYGap)));
		for (int i = 1; i < PlatformNum; i++)
		{
			float Ratio = 1.0 * i / PlatformNum;
			bool ArcDir = UKismetMathLibrary::RandomBoolFromStream(i);
			FVector Position = FindPointOnArc(StartVertex, EndVertex, true, Ratio);
			PlatformPositions.Add(Position);
			PlatformCount++;
		}
	}
	
	for (const auto& Vertex : VoronoiVertices)
	{
		PlatformPositions.Add(Vertex);
		PlatformCount++;
	}
}

UMovingPlatformComponent* APlatformPathManager::GetPlatformByIndex(int Index) const
{
	if (PlatformComponents.IsValidIndex(Index))
	{
		return PlatformComponents[Index];
	}
	return nullptr;
}