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

void UVoxelBrush::Paint(FVoxel& Voxel, FVector& VoxelPosition, const int MaterialId)
{
    const float Dist = Shape->SignedDistance(VoxelPosition, Location);
    if (Dist < 0.0f)
    {
        Voxel.Id = MaterialId;
    }
}

void UVoxelBrush::Sculpt(FVoxel& Voxel, FVector& VoxelPosition)
{
    const float Dist = Shape->SignedDistance(VoxelPosition, Location);

    Voxel.Density = Strength > 0 ? FMath::Min(Voxel.Density, Dist * Strength) :
        FMath::Max(Voxel.Density, Dist * Strength);
}