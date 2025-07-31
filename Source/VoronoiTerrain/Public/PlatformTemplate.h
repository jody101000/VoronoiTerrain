#pragma once

#include "CoreMinimal.h"
#include "PlatformPropertyManager.h"
#include "PlatformTemplate.generated.h"

UENUM(BlueprintType)
enum class ETemplateCategory : uint8
{
    EasyJump        UMETA(DisplayName = "Easy Jump"),
    MediumJump      UMETA(DisplayName = "Medium Jump"),
    HardJump        UMETA(DisplayName = "Hard Jump"),
    Vertical        UMETA(DisplayName = "Vertical"),
    Branch          UMETA(DisplayName = "Branch"),
    Special         UMETA(DisplayName = "Special")
};

USTRUCT(BlueprintType)
struct FPlatformPlacement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RelativePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPlatformType PlatformType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RelativeRotation = FRotator::ZeroRotator;

    // Connection point index (-1 if not a connection point)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ConnectionPointIndex = -1;

    FPlatformPlacement()
    {
        RelativePosition = FVector::ZeroVector;
        PlatformType = EPlatformType::Standard;
        Scale = 1.0f;
        RelativeRotation = FRotator::ZeroRotator;
        ConnectionPointIndex = -1;
    }
};

USTRUCT(BlueprintType)
struct FTemplateMetadata
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TemplateName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETemplateCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RequiredHorizontalDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RequiredVerticalDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinimumClearanceHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DifficultyRating = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBox CollisionBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsDeformation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinDeformScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDeformScale = 1.5f;

    FTemplateMetadata()
    {
        TemplateName = TEXT("Default Template");
        Category = ETemplateCategory::EasyJump;
        RequiredHorizontalDistance = 200.0f;
        RequiredVerticalDistance = 0.0f;
        MinimumClearanceHeight = 300.0f;
        DifficultyRating = 5;
        bSupportsDeformation = false;
        MinDeformScale = 0.8f;
        MaxDeformScale = 1.5f;
    }
};

USTRUCT(BlueprintType)
struct FPlatformTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTemplateMetadata Metadata;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FPlatformPlacement> Platforms;

    // Entry and exit connection points (indices into Platforms array)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EntryPointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExitPointIndex = 1;

    // Additional connection points for branching templates
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> AdditionalConnectionPoints;

    FPlatformTemplate()
    {
        EntryPointIndex = 0;
        ExitPointIndex = 1;
    }

    FBox GetLocalBounds() const;
    bool IsValidTemplate() const;
    float GetTotalDistance() const;
};