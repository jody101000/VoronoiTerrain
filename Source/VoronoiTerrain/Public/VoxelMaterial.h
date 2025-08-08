#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "VoxelMaterial.generated.h"

UCLASS(BlueprintType)
class VORONOITERRAIN_API UVoxelMaterial : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 Id;

    UPROPERTY(BlueprintReadWrite)
    UTexture2D* Texture;

    static FLinearColor Encode(const int Id);
};