#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "MovingPlatformComponent.h"
#include "FortuneAlgorithm/FortuneAlgorithm.h"
#include "MovingPlatformManager.generated.h"

class FVoronoiDiagram;

UCLASS()
class VORONOITERRAIN_API AMovingPlatformManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AMovingPlatformManager();

protected:
	virtual void BeginPlay() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void SetupPlatformAppearance(UMovingPlatformComponent* Platform);
	
	UPROPERTY()
	TArray<UMovingPlatformComponent*> PlatformComponents;
	
	UPROPERTY(BlueprintReadWrite, Category = "Moving Platform Manager", meta = (ClampMin = "0"))
	int PlatformCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Platform Manager")
	UStaticMesh* PlatformMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Platform Manager")
	UMaterial* PlatformMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Platform Manager", meta = (ClampMin = "0"))
	float InitialPlatformScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voronoi Generation")
	FBox2D VoronoiBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voronoi Generation")
	int RandomSeed;

public:	
	virtual void Tick(float DeltaTime) override;
	
	void CreatePlatforms();
	void DestroyPlatforms();
	void SetPlatformCount(int NewCount);
	
	// Compute Voronoi Diagram Using Fortune Algorithm //
	void GenerateRandomPoints();	// Write VoronoiSitePoints
	void GenerateVoronoiEdges();	// Write VoronoiEdges
	void GetPlatformTransformData();
	
	int GetPlatformCount() const { return PlatformComponents.Num(); }
	UMovingPlatformComponent* GetPlatformByIndex(int Index) const;

private:
	std::vector<Vector2> VoronoiSitePoints2D;
	TArray<TArray<TTuple<FVector, FVector>>> VoronoiEdges;
};
