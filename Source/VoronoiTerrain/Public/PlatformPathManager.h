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

class AResourcePickup;
class ALevelGoal;

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
struct FPlacedPlatformInfo
{
	GENERATED_BODY()

	UPROPERTY()
	EPlatformType Type = EPlatformType::Standard;

	UPROPERTY()
	UStaticMesh* Mesh = nullptr;

	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	UPROPERTY()
	EResourceType ResourceType = EResourceType::None;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Path Manager")
	TArray<APlatformComponent*> PlatformComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Manager")
	float PlatformSize = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Path Manager")
	TMap<EPlatformType, FPlatformMeshArray> PlatformMeshesByType;
	
	UPROPERTY(EditAnywhere, Category = "Platform Path Manager")
	TMap<EPlatformType, float> PlatformTypeWeights;

	UPROPERTY(EditAnywhere, Category="Platform Manager")
	UPlatformTypeManager* PlatformTypeManager;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiral Generation")
	int RandomSeed = 10;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource System")
	float ResourceSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Resource System")
	TMap<EResourceType, float> ResourceTypeWeights;

	UPROPERTY(EditAnywhere, Category = "Resource System")
	TSubclassOf<AResourcePickup> ResourcePickupClass;

	UPROPERTY(EditAnywhere, Category = "Resource System")
	TSubclassOf<ALevelGoal> GoalClass;

	UPROPERTY(EditAnywhere, Category = "Resource System")
	float ResourceOffsetHeight = 50.0f;


public:	
	virtual void Tick(float DeltaTime) override;

	void CreatePlatforms();
	void DestroyPlatforms();
	
	void SetupPlatformAppearance(APlatformComponent* Platform, EPlatformType Type, int MeshIndex);
	EPlatformType SelectPlatformType(int PlatformIndex, float ZPosition, EPlatformType LastPlatformType);
	UStaticMesh* SelectMeshForType(EPlatformType Type, int RandomSeed);
	
	void GeneratePlatformSpiralPositions();
	
	int GetPlatformCount() const { return PlatformComponents.Num(); }
	APlatformComponent* GetPlatformByIndex(int Index) const;

	TArray<FVector> PlatformPositions;
	int PlatformCount = 0;

private:
	UPROPERTY()
	AActor* GoalActor;

	TArray<FPlacedPlatformInfo> PlacedPlatforms;

	EResourceType SelectResourceType(int32 PlatformIndex, float ZPosition);
	void SpawnResourceOnPlatform(APlatformComponent* Platform, EResourceType ResourceType);
	void SpawnGoalAtHighestPlatform();

};
