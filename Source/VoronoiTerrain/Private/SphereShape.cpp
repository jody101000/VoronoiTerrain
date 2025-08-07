#include "SphereShape.h"

float USphereShape::SignedDistance(FVector& VoxelPosition, FVector& BrushPosition)
{
    const float Dist = FVector::Distance(VoxelPosition, BrushPosition);
    return Dist - Radius;

    // FVector RelativePos = VoxelPosition - BrushPosition;
    //
    // // Get half the side length for box extents
    // float HalfSide = Radius / 2.0f;
    //
    // // Get absolute distances from center
    // FVector AbsPos = FVector(FMath::Abs(RelativePos.X), FMath::Abs(RelativePos.Y), FMath::Abs(RelativePos.Z));
    //
    // // Calculate distance to box surface
    // FVector d = AbsPos - FVector(HalfSide, HalfSide, HalfSide);
    //
    // // Combine outside distance (positive) and inside distance (negative)
    // float OutsideDistance = FVector(FMath::Max(d.X, 0.0f), FMath::Max(d.Y, 0.0f), FMath::Max(d.Z, 0.0f)).Size();
    // float InsideDistance = FMath::Min(FMath::Max(d.X, FMath::Max(d.Y, d.Z)), 0.0f);
    //
    // return OutsideDistance + InsideDistance;
}