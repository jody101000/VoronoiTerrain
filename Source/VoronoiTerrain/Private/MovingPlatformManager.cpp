// Fill out your copyright notice in the Description page of Project Settings.


#include "MovingPlatformManager.h"
#include "MovingPlatformComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "FortuneAlgorithm/FortuneAlgorithm.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"

AMovingPlatformManager::AMovingPlatformManager()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	PlatformCount = 5;
}

void AMovingPlatformManager::BeginPlay()
{
	Super::BeginPlay();
	
	InitializePlatformTransformData();
	CreatePlatforms();
}

void AMovingPlatformManager::OnConstruction(const FTransform& Transform)
{
	FlushPersistentDebugLines(GetWorld());
	InitializePlatformTransformData();


	if (ShowDebugEdges && false)
	{
		for (const auto& EdgesPerSite : VoronoiEdges)
		{
			for (const auto& Edge : EdgesPerSite)
			{
				DrawDebugLine(GetWorld(), Edge.Get<0>() + GetActorLocation(), Edge.Get<1>() + GetActorLocation(), FColor::Blue, true, -1, 0, 5);
			}
		}
	}
	DrawDebugEdges();
	if (ShowDebugCircles)
	{
		for (int i = 0; i < PlatformCount; i++)
		{
			DrawDebugCircle(GetWorld(), PlatformPositions[i] + GetActorLocation(), PlatformRadii[i], 24, FColor::Orange, true, -1, 0, 1, FVector(0, 1, 0), FVector(1, 0, 0), false);
		}
	}
}

void AMovingPlatformManager::DrawDebugEdges()
{
	FlushPersistentDebugLines(GetWorld());
	InitializePlatformTransformData();

	FVector PositionShifts = FVector(0.0f, 0.0f, 0.0f);
	if (ShowDebugEdges)
	{
		// for (const auto& EdgesPerSite : VoronoiEdges)
		// {
		// 	for (const auto& Edge : EdgesPerSite)
		// 	{
		// 		DrawDebugLine(GetWorld(), Edge.Get<0>() + GetActorLocation() + PositionShifts, Edge.Get<1>() + GetActorLocation() + PositionShifts, FColor::Blue, true, -1, 0, 5);
		// 		DrawDebugCircle(GetWorld(), Edge.Get<0>() + GetActorLocation() + PositionShifts, 10, 24, FColor::Orange, true, -1, 0, 1, FVector(0, 1, 0), FVector(1, 0, 0), false);
		// 		DrawDebugCircle(GetWorld(), Edge.Get<1>() + GetActorLocation() + PositionShifts, 10, 24, FColor::Orange, true, -1, 0, 1, FVector(0, 1, 0), FVector(1, 0, 0), false);
		// 	}
		// 	PositionShifts += FVector(0.0f, 0.0f, 10.0f);
		// }

		for (auto Vertex : VerticesForDebug)
		{
			DrawDebugCircle(GetWorld(), Vertex + GetActorLocation() + PositionShifts, 10, 24, FColor::Orange, true, -1, 0, 2, FVector(0, 1, 0), FVector(1, 0, 0), false);
			PositionShifts += FVector(0.0f, 0.0f, 20.0f);
		}
		
		PositionShifts = FVector(0.0f, 0.0f, 0.0f);
		for (auto Edge : EdgesForDebug)
		{
			DrawDebugLine(GetWorld(), Edge.Get<0>() + GetActorLocation() + PositionShifts, Edge.Get<1>() + GetActorLocation() + PositionShifts, FColor::Blue, true, -1, 0, 5);
			PositionShifts += FVector(0.0f, 0.0f, 10.0f);
		}
	}
}




void AMovingPlatformManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update transforms
	UpdatePlatformTransformData(DeltaTime);
	UpdatePlatforms();
}

void AMovingPlatformManager::CreatePlatforms()
{
	// Clean up
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
				// UE_LOG(LogTemp, Log, TEXT("Selected mesh size: (%f, %f, %f)"), MeshSize.X, MeshSize.Y, MeshSize.Z);
			}
			

			// Set position and scale
			if (PlatformPositions.IsValidIndex(i) && PlatformRadii.IsValidIndex(i))
			{
				FVector WorldPosition = GetActorLocation() + PlatformPositions[i];
				NewPlatform->InitializePlatform(i, WorldPosition, PlatformRadii[i] / MeshSize.X * 2.0f);
			}
			else
			{
				// UE_LOG(LogTemp, Warning, TEXT("PlatformComponent_%d's position or radius is not generated correctly"), i);
			}
			
			PlatformComponents.Add(NewPlatform);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MovingPlatformManager: Created %d platforms"), PlatformComponents.Num());
}

void AMovingPlatformManager::UpdatePlatforms()
{
	// Update platforms with current Voronoi data
	for (int32 i = 0; i < PlatformCount; i++)
	{
		FVector MeshSize = FVector(1.0f);
		
		if (PlatformMesh)
		{
			MeshSize = PlatformMesh->GetBounds().GetBox().GetSize();
			UE_LOG(LogTemp, Log, TEXT("Selected mesh size: (%f, %f, %f)"), MeshSize.X, MeshSize.Y, MeshSize.Z);
		}
		
		if (PlatformComponents[i] && IsValid(PlatformComponents[i]))
		{
			FVector WorldPosition = GetActorLocation() + PlatformPositions[i];
			PlatformComponents[i]->UpdatePlatformData(WorldPosition, PlatformRadii[i] / MeshSize.X * 2.0f);
		}
	}
}

void AMovingPlatformManager::DestroyPlatforms()
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

void AMovingPlatformManager::SetupPlatformAppearance(UMovingPlatformComponent* Platform)
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

void AMovingPlatformManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		FString PropertyName = PropertyChangedEvent.Property->GetName();

		if (PropertyName == TEXT("PlatformCount") || 
			PropertyName == TEXT("RandomSeed") ||
			PropertyName == TEXT("MinHeight") ||
			PropertyName == TEXT("MaxHeight") || 
			PropertyName == TEXT("VoronoiBounds"))
		{
			CreatePlatforms();
		}
	}
}

float AMovingPlatformManager::GetRandomVelocityInRange(const FRandomStream RandomStream) const
{
	if (UKismetMathLibrary::RandomBool())
	{
		return RandomStream.FRandRange(-MaxSpeed, -MinSpeed);
	}
	else
	{
		return RandomStream.FRandRange(MinSpeed, MaxSpeed);
	}
}

void AMovingPlatformManager::GenerateRandomPoints()
{
	VoronoiSitePoints2D.clear();
	VoronoiSitePoints2DVelocity.Empty();
	PlatformHeights.Empty();
	
	FVector BoundsCenter = VoronoiBounds.GetCenter();
	FVector BoundsExtent = VoronoiBounds.GetExtent();
	const FRandomStream RandomStream(RandomSeed);
	for (int i = 0; i < PlatformCount; i++)
	{
		FVector Point = UKismetMathLibrary::RandomPointInBoundingBoxFromStream(RandomStream, BoundsCenter, BoundsExtent);
		VoronoiSitePoints2D.push_back({Point.X, Point.Y});
		PlatformHeights.Add(RandomStream.FRandRange(MinHeight, MaxHeight));
		
		// Random velocity
		float VelX = GetRandomVelocityInRange(RandomStream);
		float VelY = GetRandomVelocityInRange(RandomStream);
		VoronoiSitePoints2DVelocity.Add({VelX, VelY});
	}
}



void AMovingPlatformManager::UpdateRandomPoints(float DeltaTime)
{
	for (int i = 0; i < PlatformCount; i++)
	{
		auto& Point = VoronoiSitePoints2D[i];
		auto& Velocity = VoronoiSitePoints2DVelocity[i];
		// Update position
		Point += Velocity * DeltaTime;

		// Bounce off boundaries
		if (Point.x <= VoronoiBounds.MinX || Point.x >= VoronoiBounds.MaxX)
		{
			Velocity.x = -Velocity.x;
			Velocity.y = -Velocity.y;
			Point.x = FMath::Clamp(Point.x, VoronoiBounds.MinX, VoronoiBounds.MaxX);
		}

		if (Point.y <= VoronoiBounds.MinY || Point.y >= VoronoiBounds.MaxY)
		{
			Velocity.x = -Velocity.x;
			Velocity.y = -Velocity.y;
			Point.y = FMath::Clamp(Point.y, VoronoiBounds.MinY, VoronoiBounds.MaxY);
		}

		
	}
}


void AMovingPlatformManager::GenerateVoronoiEdges()
{
	VoronoiEdges.Empty();
	
	FortuneAlgorithm algorithm(VoronoiSitePoints2D);
	algorithm.construct();
	const double MinX = VoronoiBounds.MinX;
	const double MinY = VoronoiBounds.MinY;
	const double MaxX = VoronoiBounds.MaxX;
	const double MaxY = VoronoiBounds.MaxY;
	algorithm.bound(Box{MinX-0.05, MinY-0.05, MaxX+0.05, MaxY+0.05});
	VoronoiDiagram Diagram = algorithm.getDiagram();
	Diagram.intersect(Box{MinX, MinY, MaxX, MaxY});

	if (true)
	{
		VerticesForDebug.Empty();
		EdgesForDebug.Empty();
		std::list<VoronoiDiagram::Vertex> Vertices = Diagram.getVertices();
		std::list<VoronoiDiagram::HalfEdge> Edges = Diagram.getHalfEdges();
		for (auto Vertex : Vertices)
		{
			VerticesForDebug.Add({Vertex.point.x, Vertex.point.y, 0});
		}
		while (!Edges.empty())
		{
			VoronoiDiagram::HalfEdge& Edge = Edges.front();
			FVector Start = FVector(Edge.origin->point.x, Edge.origin->point.y, 0.0f);
			FVector End = FVector(Edge.destination->point.x, Edge.destination->point.y, 0);
			EdgesForDebug.Add(TTuple<FVector, FVector>(Start, End));
		
			Edges.pop_front();
			if (Edge.twin)
			{
				Edges.pop_front();
			}
		}
	}
	
	VoronoiEdges.Init(TArray<TTuple<FVector, FVector>>(), PlatformCount);

	for (std::size_t i = 0; i < PlatformCount; ++i)
	{
		const VoronoiDiagram::Site* site = Diagram.getSite(i);
		Vector2 center = site->point;
		VoronoiDiagram::Face* face = site->face;
		VoronoiDiagram::HalfEdge* halfEdge = face->outerComponent;
		if (halfEdge == nullptr)
			continue;
		while (halfEdge->prev != nullptr)
		{
			halfEdge = halfEdge->prev;
			if (halfEdge == face->outerComponent)
				break;
		}
		VoronoiDiagram::HalfEdge* start = halfEdge;
		while (halfEdge != nullptr)
		{
			if (halfEdge->origin != nullptr && halfEdge->destination != nullptr)
			{
				Vector2 _origin = (halfEdge->origin->point - center) + center;
				FVector origin(_origin.x, _origin.y, 0);
				Vector2 _destination = (halfEdge->destination->point - center) + center;
				FVector destination(_destination.x, _destination.y, 0);
				VoronoiEdges[i].Add(TTuple<FVector, FVector>(origin, destination));
			}
			halfEdge = halfEdge->next;
			if (halfEdge == start)
				break;
		}
	}
}

void AMovingPlatformManager::InitializePlatformTransformData()
{
	GenerateRandomPoints();
	GenerateVoronoiEdges();

	GeneratePlatformPositions();
	GeneratePlatformRadii();
}

void AMovingPlatformManager::UpdatePlatformTransformData(float DeltaTime)
{
	UpdateRandomPoints(DeltaTime);
	GenerateVoronoiEdges();

	GeneratePlatformPositions();
	GeneratePlatformRadii();
}


void AMovingPlatformManager::GeneratePlatformPositions()
{
	PlatformPositions.Empty();
	
	for (int i = 0; i < PlatformCount; i++)
	{
		const auto& EdgesPerSite = VoronoiEdges[i];
		float Area = 0;
		float CenterX = 0;
		float CenterY = 0;
		for (const auto& Edge : EdgesPerSite)
		{
			float Value = Edge.Get<0>().X * Edge.Get<1>().Y - Edge.Get<1>().X * Edge.Get<0>().Y;
			CenterX += (Edge.Get<0>().X + Edge.Get<1>().X) * Value;
			CenterY += (Edge.Get<0>().Y + Edge.Get<1>().Y) * Value;
			Area += Edge.Get<0>().X * Edge.Get<1>().Y - Edge.Get<1>().X * Edge.Get<0>().Y;
		}
		// Area *= 0.5;
		CenterX /= 3.0 * Area;
		CenterY /= 3.0 * Area;
		PlatformPositions.Add(FVector(CenterX, CenterY, PlatformHeights[i]));
		
		// UE_LOG(LogTemp, Warning, TEXT("PlatformPositions at (%f, %f)"), CenterX, CenterY);
	}
}

void AMovingPlatformManager::GeneratePlatformRadii()
{
	PlatformRadii.Empty();
	
	for (int i = 0; i < PlatformCount; i++)
	{
		TArray<TTuple<FVector, FVector>>& SiteEdges = VoronoiEdges[i];
		const FVector CenterPos = FVector(PlatformPositions[i].X, PlatformPositions[i].Y, 0);
		float MinDistance = MAX_FLT;	// Distance from the center to the closest edge
		for (const auto& edge : SiteEdges)
		{
			MinDistance = std::min(MinDistance, UKismetMathLibrary::GetPointDistanceToSegment(CenterPos, edge.Get<0>(), edge.Get<1>()));
		}
		PlatformRadii.Add(MinDistance);
	}
}
