#include "VoxelSystem/VoxelShape.h"

float UVoxelShape::SignedDistance(FVector& VoxelPosition, FVector& BrushPosition)
{
    const float Dist = FVector::Distance(VoxelPosition, BrushPosition);
    return Dist - Radius;
}