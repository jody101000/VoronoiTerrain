#pragma once
#include "VoxelData.generated.h"

USTRUCT(Blueprintable)
struct FVoxel
{
    GENERATED_BODY()

    FVoxel() : Density(1.0f), Id(0), MaterialId(0) {}
    FVoxel(const float Density, const int Id, const int MaterialId) : Density(Density), Id(Id), MaterialId(MaterialId) {}

    UPROPERTY(BlueprintReadOnly)
    float Density;

    UPROPERTY(BlueprintReadOnly)
    int Id;

    UPROPERTY(BlueprintReadOnly)
    int MaterialId;
};