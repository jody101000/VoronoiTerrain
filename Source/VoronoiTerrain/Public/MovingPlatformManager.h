#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "MovingPlatformComponent.h"
#include "FortuneAlgorithm/FortuneAlgorithm.h"
#include "MovingPlatformManager.generated.h"

class FVoronoiDiagram;

USTRUCT()
struct FVoronoiBounds
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	float MinX = 0.0f;

	UPROPERTY(EditAnywhere)
	float MinY = 0.0f;

	UPROPERTY(EditAnywhere)
	float MaxX = 1.0f;

	UPROPERTY(EditAnywhere)
	float MaxY = 1.0f;

	FVoronoiBounds() = default;
	FVoronoiBounds(float InMinX, float InMinY, float InMaxX, float InMaxY)
		: MinX(InMinX), MinY(InMinY), MaxX(InMaxX), MaxY(InMaxY) {}

	FVector GetCenter() const { return FVector(MaxX + MinX, MaxY + MinY, 0) / 2.0f; }
	FVector GetExtent() const { return FVector(MaxX - MinX, MaxY - MinY, 0) / 2.0f; }
};

UCLASS()
class VORONOITERRAIN_API AMovingPlatformManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AMovingPlatformManager();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void SetupPlatformAppearance(UMovingPlatformComponent* Platform);
	
	UPROPERTY()
	TArray<UMovingPlatformComponent*> PlatformComponents;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Platform Manager", meta = (ClampMin = "0"))
	int PlatformCount = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Platform Manager")
	UStaticMesh* PlatformMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Platform Manager")
	UMaterial* PlatformMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Platform Manager")
	float MinHeight = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Platform Manager")
	float MaxHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voronoi Generation")
	float MinSpeed = 5.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voronoi Generation")
	float MaxSpeed = 10.0f;
	
	UPROPERTY(EditAnywhere, Category = "Voronoi Generation")
	FVoronoiBounds VoronoiBounds = FVoronoiBounds(-500.0f, -500.0f, 500.0f, 500.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voronoi Generation")
	int RandomSeed = 10;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool ShowDebugEdges = false;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool ShowDebugCircles = true;

public:	
	virtual void Tick(float DeltaTime) override;
	
	void CreatePlatforms();
	void UpdatePlatforms();
	void DestroyPlatforms();
	
	// Compute Voronoi Diagram Using Fortune Algorithm //
	void GenerateRandomPoints();	// Write VoronoiSitePoints
	void UpdateRandomPoints(float DeltaTime);
	void GenerateVoronoiEdges();	// Write VoronoiEdges
	void InitializePlatformTransformData();
	void UpdatePlatformTransformData(float DeltaTime);
	float GetRandomVelocityInRange(const FRandomStream RandomStream) const;
	
	void GeneratePlatformPositions();
	void GeneratePlatformRadii();

	TArray<FVector> PlatformPositions;
	TArray<float> PlatformRadii;

private:
	std::vector<Vector2> VoronoiSitePoints2D;
	TArray<float> PlatformHeights;
	TArray<TArray<TTuple<FVector, FVector>>> VoronoiEdges;
	TArray<Vector2> VoronoiSitePoints2DVelocity;
};
