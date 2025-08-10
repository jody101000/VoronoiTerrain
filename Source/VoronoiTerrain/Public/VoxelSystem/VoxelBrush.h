#pragma once
#include "CoreMinimal.h"
#include "VoxelShape.h"
#include "VoxelData.h"
#include "UObject/Object.h"
#include "VoxelBrush.generated.h"

UCLASS(Blueprintable)
class VORONOITERRAIN_API UVoxelBrush : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    FVector Location;

    UPROPERTY(BlueprintReadWrite)
    UVoxelShape* Shape;

    UPROPERTY(BlueprintReadWrite)
    float Strength = 1.0;

    UPROPERTY(BlueprintReadWrite)
    int32 MaterialId = 0;

    UPROPERTY(BlueprintReadWrite)
    float VoxelSize = 20.0f;

    UVoxelBrush();
    UVoxelBrush(UVoxelShape* Shape);

    void Paint(FVoxel& Voxel, FVector& VoxelPosition);
    void Sculpt(FVoxel& Voxel, FVector& VoxelPosition);
};