#include "VoxelSystem/VoxelBrush.h"
#include "VoxelSystem/VoxelShape.h"

UVoxelBrush::UVoxelBrush()
{
    Shape = NewObject<UVoxelShape>();
}

UVoxelBrush::UVoxelBrush(UVoxelShape* InShape)
{
    Shape = InShape;
}

void UVoxelBrush::Sculpt(FVoxel& Voxel, FVector& VoxelPosition)
{
    // Distance from brush surface. Positive if outside surface
    const float Dist = Shape->SignedDistance(VoxelPosition, Location);

    if (Strength > 0) // Sculpting
    {
        if (Voxel.Density > Dist)
        {
            Voxel.Density = Dist;
        }
        else if (Voxel.Density > 0)
        {
            float UpdateSize = 1.0f * 32;
            float UpdateWeight = FMath::SmoothStep(0.0f, 1.0f, (FMath::Abs(Dist) / UpdateSize)); 
            Voxel.Density = FMath::Lerp(Dist, Voxel.Density, UpdateWeight);
        }
    }
    else // Erasing
    {
        Voxel.Density = FMath::Max(Voxel.Density, -Dist);
    }
}

void UVoxelBrush::Paint(FVoxel& Voxel, FVector& VoxelPosition)
{
    Voxel.MaterialId = MaterialId;
}