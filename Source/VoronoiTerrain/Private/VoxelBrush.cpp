#include "VoxelBrush.h"
#include "SphereShape.h"

UVoxelBrush::UVoxelBrush()
{
    Shape = NewObject<USphereShape>();
}

UVoxelBrush::UVoxelBrush(UVoxelShape* InShape)
{
    Shape = InShape;
}

void UVoxelBrush::Sculpt(FVoxel& Voxel, FVector& VoxelPosition)
{
    const float Dist = Shape->SignedDistance(VoxelPosition, Location);

    float OldDensity = Voxel.Density;

    Voxel.Density = Strength > 0 ? FMath::Min(Voxel.Density, Dist * Strength) :
        FMath::Max(Voxel.Density, Dist * Strength);
}

void UVoxelBrush::Paint(FVoxel& Voxel, FVector& VoxelPosition)
{
    const float Dist = Shape->SignedDistance(VoxelPosition, Location);
    
    if (Dist < 0.0f && Strength != -1)
    {
        Voxel.MaterialId = MaterialId;
    }
}