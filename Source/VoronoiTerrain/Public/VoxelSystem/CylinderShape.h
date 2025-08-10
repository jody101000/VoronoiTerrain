#pragma once
#include "CoreMinimal.h"
#include "VoxelShape.h"
#include "UObject/Object.h"
#include "CylinderShape.generated.h"

UCLASS(Blueprintable)
class VORONOITERRAIN_API UCylinderShape : public UVoxelShape
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    float Radius = 2.0f;

    virtual float SignedDistance(FVector& VoxelPosition, FVector& BrushPosition) override;

private:
    float GetHeight() const { return Radius * 0.2f; } // Height = 1/5 of radius
};