#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VoxelShape.generated.h"

UCLASS(Blueprintable)
class VORONOITERRAIN_API UVoxelShape : public UObject
{
    GENERATED_BODY()
public:
    float SignedDistance(FVector& VoxelPosition, FVector& BrushPosition);

    UPROPERTY(BlueprintReadWrite)
    float Radius = 2.0;
};