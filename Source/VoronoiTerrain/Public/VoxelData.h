#pragma once
#include "VoxelData.generated.h"

USTRUCT(Blueprintable)
struct FVoxel
{
    GENERATED_BODY()

    FVoxel() : Density(1.0f), Id(0) {}
    FVoxel(const float Density, const int Id) : Density(Density), Id(Id) {}

    UPROPERTY(BlueprintReadOnly)
    float Density;

    UPROPERTY(BlueprintReadOnly)
    int Id;
};