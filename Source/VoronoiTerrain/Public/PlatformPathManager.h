// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "PlatformTypeManager.h"
#include "PlatformComponent.h"
#include "FortuneAlgorithm/FortuneAlgorithm.h"
#include "PlatformPathManager.generated.h"

USTRUCT(BlueprintType)
struct FPlatformMeshArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Meshes")
	TArray<UStaticMesh*> Meshes;

	FPlatformMeshArray()
	{
		Meshes.Empty();
	}
};

USTRUCT()
struct FSectionSize
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	float SizeX = 500.0f;

	UPROPERTY(EditAnywhere)
	float SizeZ = 500.0f;

	FSectionSize() = default;
	FSectionSize(float InSizeX, float InSizeZ)
		: SizeX(InSizeX), SizeZ(InSizeZ) {}

	FVector GetCenter() const { return FVector(SizeX, 0, SizeZ) / 2.0f; }
	FVector GetExtent() const { return FVector(SizeX, 0, SizeZ) / 2.0f; }
};

USTRUCT()
struct FGapSize
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	float MinXY = 0.0f;

	UPROPERTY(EditAnywhere)
	float MaxXY = 1.0f;
	
	UPROPERTY(EditAnywhere)
	float MinZ = 0.0f;

	UPROPERTY(EditAnywhere)
	float MaxZ = 1.0f;

	FGapSize() = default;
	FGapSize(float InMinXY, float InMaxXY, float InMinZ, float InMaxZ)
		: MinXY(InMinXY), MaxXY(InMaxXY), MinZ(InMinZ), MaxZ(InMaxZ) {}

};

USTRUCT()
struct FPlacedPlatformInfo
{
	GENERATED_BODY()

	UPROPERTY()
	EPlatformType Type = EPlatformType::Standard;

	UPROPERTY()
	UStaticMesh* Mesh = nullptr;

	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	FPlacedPlatformInfo() = default;
	FPlacedPlatformInfo(EPlatformType InType, UStaticMesh* InMesh, FVector InPosition)
		: Type(InType), Mesh(InMesh), Position(InPosition) {}
};


UCLASS()
class VORONOITERRAIN_API APlatformPathManager : public AActor
{
	GENERATED_BODY()
	
public:	
	APlatformPathManager();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	// virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Path Manager")
	TArray<APlatformComponent*> PlatformComponents;
	
	UPROPERTY(EditAnywhere, Category = "Platform Path Manager")
	FGapSize GapSize;

	UPROPERTY(EditAnywhere, Category = "Platform Path Manager")
	float MaxXNoise;
	
	UPROPERTY(EditAnywhere, Category = "Platform Manager", meta = (ClampMin = "-180.0", ClampMax = "180.0", UIMin = "-180.0", UIMax = "180.0"))
	float MaxRotationAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Manager")
	float PlatformSize = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Path Manager")
	TMap<EPlatformType, FPlatformMeshArray> PlatformMeshesByType;
	
	UPROPERTY(EditAnywhere, Category = "Platform Path Manager")
	TMap<EPlatformType, float> PlatformTypeWeights;

	UPROPERTY(EditAnywhere, Category="Platform Manager")
	UPlatformTypeManager* PlatformTypeManager;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voronoi Generation", meta = (ClampMin = "1", UIMin = "1"))
	int SiteCount = 10;
	
	UPROPERTY(EditAnywhere, Category = "Voronoi Generation")
	FSectionSize SectionSize;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voronoi Generation")
	int RandomSeed = 10;

	UPROPERTY(EditAnywhere, Category = "Voronoi Generation")
	float MinAngleDegree = 30;
	
	UPROPERTY(EditAnywhere, Category = "Voronoi Generation")
	float MaxAngleDegree = 45;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool ShowDebugEdges = false;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool ShowDebugCircles = false;


	UPROPERTY(EditAnywhere, Category = "Spiral Generation", meta = (ClampMin = "50.0"))
	float SpiralRadius = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Spiral Generation", meta = (ClampMin = "100.0"))
	float SpiralHeight = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Spiral Generation", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float SpiralTurns = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Spiral Generation", meta = (ClampMin = "4", ClampMax = "50"))
	int32 PlatformsPerTurn = 8;


public:	
	virtual void Tick(float DeltaTime) override;

	void CreatePlatforms();
	void DestroyPlatforms();

	// Compute Voronoi Diagram Using Fortune Algorithm //
	void GeneratePathNet();
	void GenerateRandomPoints();	// Write VoronoiSitePoints
	void GenerateVoronoiEdges();	// Write VoronoiEdges
	void InclinedVoronoiEdges(); // Shift vertices of 2D voronoi Diagram to 3D path intersections

	void GenerateFlatPathNet();
	
	void SetupPlatformAppearance(APlatformComponent* Platform, EPlatformType Type, int MeshIndex);
	EPlatformType SelectPlatformType(int PlatformIndex, float ZPosition, EPlatformType LastPlatformType);
	UStaticMesh* SelectMeshForType(EPlatformType Type, int RandomSeed);
	
	void OrderVerticesByHeight();
	void OrderEdgesByHeight();
	bool CheckPlatformCollision(const FVector& Position, UStaticMesh* Mesh, float Scale);
	float CalculateMinimumSpacing(UStaticMesh* Mesh1, UStaticMesh* Mesh2, float Scale);
	
	void GeneratePlatformPositions();
	void CheckPlatformInfoCollisions();

	void GeneratePlatformSpiralPositions();
	
	int GetPlatformCount() const { return PlatformComponents.Num(); }
	APlatformComponent* GetPlatformByIndex(int Index) const;

	TArray<FVector> PlatformPositions;
	int PlatformCount = 0;

private:

	TArray<TTuple<int, int>> ConvertEdgesToIndices(const TArray<FVector>& Vertices, const TArray<TTuple<FVector, FVector>>& PositionEdges) const;
	
	std::vector<Vector2> VoronoiSitePoints2D;
	TArray<TTuple<int, int>> VoronoiEdges;
	TArray<FVector> VoronoiVertices;

	TArray<int> SortedVertexIndices;

	TArray<FPlacedPlatformInfo> PlacedPlatforms;

};
