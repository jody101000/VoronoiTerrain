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
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	GapSize = FGapSize(100.0f, 200.0f, 100.0f, 200.0f);
}

void APlatformPathManager::BeginPlay()
{
	Super::BeginPlay();

	GeneratePlatformPositions();
	CreatePlatforms();
}

void APlatformPathManager::OnConstruction(const FTransform& Transform)
{
	FlushPersistentDebugLines(GetWorld());
	GeneratePathNet();
	
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
}


void APlatformPathManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

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

		if (NewPlatform)
		{
			NewPlatform->RegisterComponent();
			NewPlatform->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
			SetupPlatformAppearance(NewPlatform);

			FVector MeshSize = FVector(1.0f);
			if (PlatformMesh)
			{
				MeshSize = PlatformMesh->GetBounds().GetBox().GetSize();
				UE_LOG(LogTemp, Log, TEXT("Selected mesh size: (%f, %f, %f)"), MeshSize.X, MeshSize.Y, MeshSize.Z);
			}

			// Set position
			if (PlatformPositions.IsValidIndex(i))
			{
				FVector WorldPosition = GetActorLocation() + PlatformPositions[i];
				// ToDo: scaling for both X and Y axis
				NewPlatform->InitializePlatform(i, WorldPosition, PlatformSize / MeshSize.X * 2.0f);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("PlatformComponent_%d's position or radius is not generated correctly"), i);
			}
			
			PlatformComponents.Add(NewPlatform);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("PlatformPathManager: Created %d platforms"), PlatformComponents.Num());
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
}

void APlatformPathManager::SetupPlatformAppearance(UMovingPlatformComponent* Platform)
{
	if (!Platform) return;
	
	if (PlatformMesh)
	{
		Platform->SetStaticMesh(PlatformMesh);
	}
	
	if (PlatformMaterial)
	{
		Platform->SetMaterial(0, PlatformMaterial);
	}
}

void APlatformPathManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		FString PropertyName = PropertyChangedEvent.Property->GetName();

		if (PropertyName == TEXT("GapSize") || 
			PropertyName == TEXT("PlatformSize") ||
			PropertyName == TEXT("SectionSize") ||
			PropertyName == TEXT("RandomSeed"))
		{
			CreatePlatforms();
		}
	}
}

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
		auto PositionEdge = TTuple<FVector, FVector>(Start, End);
		VoronoiPositionEdges.Add(PositionEdge);
		Edges.pop_front();
		if (HalfEdge.twin)
		{
			Edges.pop_front();
		}
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
	float MinAngle = PI / 6.0f;
	float MaxAngle = PI / 4.0f;

	for (int i = 0; i < VoronoiEdges.Num(); i++)
	{
		const auto& Edge = VoronoiEdges[i];
		int v1 = Edge.Get<0>();
		int v2 = Edge.Get<1>();
		
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
	if (!ShowNoIncline) InclinedVoronoiEdges();
}



// Generate positions from edges
void APlatformPathManager::GeneratePlatformPositions()
{
	// for (const auto& Edge : VoronoiEdges)
	// {
	// 	int v1 = Edge.Get<0>();
	// 	int v2 = Edge.Get<1>();
	// 	const FVector& StartVertex = VoronoiVertices[v1];
	// 	const FVector& EndVertex = VoronoiVertices[v2];
	// 	float EdgeLength = FVector::Dist(StartVertex, EndVertex);
	//
	// 	// according to density, lerp
	// 	const FRandomStream RandomStream(0);
	// 	float XYGap = UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStream, GapSize.MinXY, GapSize.MaxXY);
	// 	float ZGap = UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStream, GapSize.MinZ, GapSize.MaxZ);
	// 	int PlatformNum = static_cast<int>(ceil(EdgeLength / FMath::Min(ZGap, XYGap)));
	// 	for (int i = 0; i < PlatformNum; i++)
	// 	{
	// 		
	// 	}
	// 	FVector Position = FMath::Lerp(StartVertex, EndVertex, 0.5);
	// }
	//
	// PlatformPositions.Add(FVector(0,0,0));
}

void APlatformPathManager::GeneratePlatformSize()
{
	
}

UMovingPlatformComponent* APlatformPathManager::GetPlatformByIndex(int Index) const
{
	if (PlatformComponents.IsValidIndex(Index))
	{
		return PlatformComponents[Index];
	}
	return nullptr;
}

