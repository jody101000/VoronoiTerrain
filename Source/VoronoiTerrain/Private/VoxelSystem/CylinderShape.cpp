#include "VoxelSystem/CylinderShape.h"

float UCylinderShape::SignedDistance(FVector& VoxelPosition, FVector& BrushPosition)
{
    FVector Diff = VoxelPosition - BrushPosition;

    // Distance in XY plane
    float DistXY = FMath::Sqrt(Diff.X * Diff.X + Diff.Y * Diff.Y);

    // Distance in Z direction
    float DistZ = FMath::Abs(Diff.Z);

    // Height is 1/5 of radius
    float Height = GetHeight();

    // Signed distance to cylinder
    float DistToRadius = DistXY - Radius;
    float DistToHeight = DistZ - Height * 0.5f;

    return FMath::Max(DistToRadius, DistToHeight);
}