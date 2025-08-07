#pragma once
#include "CoreMinimal.h"
#include "VoxelPhysicsTypes.generated.h"

UENUM(BlueprintType)
enum class EVoxelPhysicsType : uint8
{
    Standard    UMETA(DisplayName = "Standard"),
    Bouncy      UMETA(DisplayName = "Bouncy"),
    Slippery    UMETA(DisplayName = "Slippery")
};

USTRUCT(BlueprintType)
struct VORONOITERRAIN_API FVoxelPhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrakingDeceleration = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BounceCoefficient = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector VelocityMultiplier = FVector(1.0f, 1.0f, 1.0f);

    FVoxelPhysicsProperties() = default;

    static FVoxelPhysicsProperties GetPropertiesForType(EVoxelPhysicsType Type)
    {
        FVoxelPhysicsProperties Props;
        switch (Type)
        {
        case EVoxelPhysicsType::Bouncy:
            Props.BounceCoefficient = 2.0f;
            break;
        case EVoxelPhysicsType::Slippery:
            Props.BrakingDeceleration = 400.0f;
            Props.BounceCoefficient = 0.1f;
            break;
        default: // Standard
            break;
        }
        return Props;
    }
};