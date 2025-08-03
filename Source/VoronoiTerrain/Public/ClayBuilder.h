#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldManager.h"
#include "ClayBuilder.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class VORONOITERRAIN_API UClayBuilder : public UActorComponent
{
    GENERATED_BODY()

public:
    UClayBuilder();

    void StartBuildClay();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 100.0f, ClampMax = 2000.0f))
    float MaxBuildDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInstance* VoxelMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrushRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrushStrength = 1.0f;

    bool GetMouseWorldPosition(FVector& MouseWorldPosition) const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UVoxelWorldManager* VoxelWorldManager;
};