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
	InitialPlatformScale = 1.0f;
}

void AMovingPlatformManager::BeginPlay()
{
	Super::BeginPlay();

	CreatePlatforms();
}

void AMovingPlatformManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update transforms
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

			FVector DefaultPosition = GetActorLocation() + FVector(i * 200.0f, 0.0f, 0.0f);
			NewPlatform->InitializePlatform(i, DefaultPosition, InitialPlatformScale);
			
			PlatformComponents.Add(NewPlatform);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MovingPlatformManager: Created %d platforms"), PlatformComponents.Num());
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

void AMovingPlatformManager::SetPlatformCount(int NewCount)
{
	if (NewCount != PlatformCount && NewCount > 0)
	{
		PlatformCount = NewCount;
		CreatePlatforms();
	}
}

UMovingPlatformComponent* AMovingPlatformManager::GetPlatformByIndex(int Index) const
{
	if (PlatformComponents.IsValidIndex(Index))
	{
		return PlatformComponents[Index];
	}
	return nullptr;
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
			PropertyName == TEXT("RandomSeed"))
		{
			CreatePlatforms();
		}
	}
}

void AMovingPlatformManager::GenerateRandomPoints()
{
	FVector BoundsCenter = FVector(VoronoiBounds.GetCenter(), 0);
	FVector BoundsExtent = FVector(VoronoiBounds.GetExtent(), 0);
	const FRandomStream RandomStream(RandomSeed);
	for (int i = 0; i < PlatformCount; i++)
	{
		FVector point = UKismetMathLibrary::RandomPointInBoundingBoxFromStream(RandomStream, BoundsCenter, BoundsExtent);
		VoronoiSitePoints2D.push_back({point.X, point.Y});
	}
}

void AMovingPlatformManager::GenerateVoronoiEdges()
{
	FortuneAlgorithm algorithm(VoronoiSitePoints2D);
	algorithm.construct();
	double MinX = VoronoiBounds.Min.X;
	double MinY = VoronoiBounds.Min.Y;
	double MaxX = VoronoiBounds.Max.X;
	double MaxY = VoronoiBounds.Max.Y;
	algorithm.bound(Box{MinX-0.05, MinY-0.05, MaxX+0.05, MaxY+0.05});
	VoronoiDiagram diagram = algorithm.getDiagram();
	diagram.intersect(Box{MinX, MinY, MaxX, MaxY});
	
	VoronoiEdges.Init(TArray<TTuple<FVector, FVector>>(), PlatformCount);

	for (std::size_t i = 0; i < PlatformCount; ++i)
	{
		const VoronoiDiagram::Site* site = diagram.getSite(i);
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

void AMovingPlatformManager::GetPlatformTransformData()
{
	VoronoiEdges.Empty();

	GenerateRandomPoints();
	GenerateVoronoiEdges();

	
	
}


