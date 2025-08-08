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

    if (Strength > 0 && Voxel.Density < 0.0f && OldDensity >= 0.0f)
    {
        Voxel.Id = MaterialId;
    }
}