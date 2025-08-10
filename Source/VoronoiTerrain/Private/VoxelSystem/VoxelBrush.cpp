#include "VoxelSystem/VoxelBrush.h"
#include "VoxelSystem/SphereShape.h"

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

    if (Strength > 0) // Sculpting
    {
        // Calculate blend weight based on distance from brush edge
        float BlendRadius = 1.0f * 32; // Blend zone thickness
        float BlendWeight = FMath::SmoothStep(0.0f, 1.0f,
            (FMath::Abs(Dist) / BlendRadius));

        // Lerp between direct assignment and Min operation
        float DirectValue = Dist;
        float MinValue = FMath::Min(Voxel.Density, Dist);
        //Voxel.Density = FMath::Lerp(DirectValue, MinValue, BlendWeight);
        float Blended = FMath::Lerp(DirectValue, MinValue, BlendWeight);

        // Preserve existing solid material (union operation)
        if (Voxel.Density < 0) {
            Voxel.Density = FMath::Min(Voxel.Density, Blended);
        }
        else {

            Voxel.Density = Blended;
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


    //const float Dist = Shape->SignedDistance(VoxelPosition, Location);

    //if (Strength > 0) // Sculpting
    //{
    //    // Calculate blend weight based on distance from brush edge
    //    float BlendRadius = 1.0f * 32; // Blend zone thickness
    //    float BlendWeight = FMath::SmoothStep(0.0f, 1.0f,
    //        (FMath::Abs(Dist) / BlendRadius));

    //    if (Dist < BlendRadius)
    //    {
    //        if (Dist < 0.0f)
    //        {
    //            float DirectValue = MaterialId;
    //            float BlendedValue = FMath::Lerp(DirectValue, Voxel.MaterialId, BlendWeight * 0.5f);
    //            Voxel.MaterialId = BlendedValue;
    //        }
    //        else
    //        {
    //            float InfluenceStrength = 1.0f - (Dist / BlendRadius);
    //            Voxel.MaterialId = FMath::Lerp(Voxel.MaterialId, MaterialId, InfluenceStrength * 0.3f);
    //        }
    //    }
    //}
}