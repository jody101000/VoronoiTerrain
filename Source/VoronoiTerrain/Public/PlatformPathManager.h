// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "MovingPlatformComponent.h"
#include "FortuneAlgorithm/FortuneAlgorithm.h"
#include "PlatformPathManager.generated.h"

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


UCLASS()
class VORONOITERRAIN_API APlatformPathManager : public AActor
{
	GENERATED_BODY()
	
public:	
	APlatformPathManager();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void SetupPlatformAppearance(UMovingPlatformComponent* Platform);
	
	UPROPERTY()
	TArray<UMovingPlatformComponent*> PlatformComponents;
	
	UPROPERTY(EditAnywhere, Category = "Platform Path Manager")
	FGapSize GapSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Path Manager")
	float PlatformSize = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Path Manager")
	UStaticMesh* PlatformMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Path Manager")
	UMaterial* PlatformMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voronoi Generation")
	int SiteCount = 10;
	
	UPROPERTY(EditAnywhere, Category = "Voronoi Generation")
	FSectionSize SectionSize;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voronoi Generation")
	int RandomSeed = 10;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool ShowDebugEdges = false;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool ShowNoIncline = false;

public:	
	virtual void Tick(float DeltaTime) override;

	void CreatePlatforms();
	void DestroyPlatforms();

	// Compute Voronoi Diagram Using Fortune Algorithm //
	void GeneratePathNet();
	void GenerateRandomPoints();	// Write VoronoiSitePoints
	void GenerateVoronoiEdges();	// Write VoronoiEdges
	void InclinedVoronoiEdges(); // Shift vertices of 2D voronoi Diagram to 3D path intersections
	
	void GeneratePlatformPositions();

	
	void GeneratePlatformSize();
	
	int GetPlatformCount() const { return PlatformComponents.Num(); }
	UMovingPlatformComponent* GetPlatformByIndex(int Index) const;

	TArray<FVector> PlatformPositions;
	int PlatformCount = 0;

private:

	TArray<TTuple<int, int>> ConvertEdgesToIndices(const TArray<FVector>& Vertices, const TArray<TTuple<FVector, FVector>>& PositionEdges) const;
	
	std::vector<Vector2> VoronoiSitePoints2D;
	TArray<TTuple<int, int>> VoronoiEdges;
	TArray<FVector> VoronoiVertices;
};
