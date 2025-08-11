#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformComponent.h"

#include "PlatformPathManager.generated.h"

class UStaticMesh;
class UPlatformTypeManager;
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
	int32 ResourceIndex = -1;

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


	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Path Manager")
	int RandomSeed = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Path Manager")
	TMap<EPlatformType, FPlatformMeshArray> PlatformMeshesByType;

	UPROPERTY(EditAnywhere, Category = "Platform Path Manager")
	TMap<EPlatformType, float> PlatformTypeWeights;

	UPROPERTY(EditAnywhere, Category = "Platform Manager")
	UPlatformTypeManager* PlatformTypeManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Manager")
	float PlatformSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Linear Generation")
	FVector StartPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Linear Generation")
	FVector EndPosition = FVector(2000.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Linear Generation", meta = (ClampMin = "1", ClampMax = "100"))
	int32 PlatformCount = 20;

	UPROPERTY(EditAnywhere, Category = "Linear Generation")
	FVector2D PlatformDistanceRange = FVector2D(100.0f, 200.0f);

	UPROPERTY(EditAnywhere, Category = "Linear Generation")
	FVector2D OrthogonalShiftRange = FVector2D(-100.0f, 100.0f);

	UPROPERTY(EditAnywhere, Category = "Resource System")
	TArray<TSubclassOf<AResourcePickup>> ResourcePickupClasses;

	UPROPERTY(EditAnywhere, Category = "Resource System")
	TSubclassOf<ALevelGoal> GoalClass;

	UPROPERTY(EditAnywhere, Category = "Resource System")
	float ResourceOffsetHeight = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool ShowDebugCircles = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:	
	void GenerateLinearPlatformPositions();

	void CreatePlatforms();
	void DestroyPlatforms();
	
	EPlatformType SelectPlatformType(float StandardWeight, float MoveWeight);
	UStaticMesh* SelectMeshForType(EPlatformType Type, int RandomSeed);

	void SpawnGoalAtHighestPlatform();

	TArray<FPlacedPlatformInfo> PlacedPlatforms;
	TArray<APlatformComponent*> PlatformComponents;

	void SpawnResourcesOnPlatforms();
	void SpawnResourceOnPlatform(APlatformComponent* Platform, int32 ResourceTypeIndex);
};
