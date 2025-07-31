#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlatformTemplate.h"
#include "TemplateLibrary.generated.h"

USTRUCT()
struct FTemplateSelectionCriteria
{
    GENERATED_BODY()

    UPROPERTY()
    float RequiredHorizontalDistance;

    UPROPERTY()
    float RequiredVerticalDistance;

    UPROPERTY()
    int32 MaxDifficulty;

    UPROPERTY()
    TArray<ETemplateCategory> PreferredCategories;

    FTemplateSelectionCriteria()
    {
        RequiredHorizontalDistance = 200.0f;
        RequiredVerticalDistance = 0.0f;
        MaxDifficulty = 10;
    }
};

USTRUCT(BlueprintType)
struct FTemplateEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPlatformTemplate Template;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETemplateCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TemplateID;

    FTemplateEntry()
    {
        Category = ETemplateCategory::EasyJump;
        TemplateID = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class VORONOITERRAIN_API UTemplateLibrary : public UObject
{
    GENERATED_BODY()

public:
    UTemplateLibrary();

    // Template management
    UFUNCTION(Category = "Template Library")
    void RegisterTemplate(const FPlatformTemplate& Template);

    UFUNCTION(Category = "Template Library")
    TArray<FPlatformTemplate> GetTemplatesByCategory(ETemplateCategory Category) const;

    UFUNCTION(Category = "Template Library")
    FPlatformTemplate SelectTemplate(const FTemplateSelectionCriteria& Criteria, int32 RandomSeed = 0) const;

    // Template creation helpers
    UFUNCTION(Category = "Template Library")
    static FPlatformTemplate CreateSimpleJumpTemplate(float GapDistance, EPlatformType PlatformType = EPlatformType::Standard);

    UFUNCTION(Category = "Template Library")
    static FPlatformTemplate CreateSteppingStoneTemplate(float TotalDistance, int32 StoneCount = 3);

    UFUNCTION(Category = "Template Library")
    static FPlatformTemplate CreateVerticalClimbTemplate(float Height, int32 PlatformCount = 4);

    UFUNCTION(Category = "Template Library")
    static FPlatformTemplate CreateBranchTemplate(float BranchAngle = 45.0f);

    // Template transformation
    UFUNCTION(Category = "Template Library")
    static TArray<FVector> TransformTemplateToWorld(
        const FPlatformTemplate& Template,
        const FVector& EntryWorldPos,
        const FVector& ExitWorldPos,
        float DeformScale = 1.0f
    );

protected:
    // Store all templates in a single array
    UPROPERTY(EditAnywhere, Category = "Template Library")
    TArray<FTemplateEntry> AllTemplates;

    // Index for quick category lookup - renamed to avoid conflict
    UPROPERTY()
    TMap<ETemplateCategory, FString> TemplateCategoryIndices;

    void InitializeDefaultTemplates();
    float CalculateTemplateScore(const FPlatformTemplate& Template, const FTemplateSelectionCriteria& Criteria) const;

    void RebuildCategoryIndices();
    TArray<int32> GetIndicesForCategory(ETemplateCategory Category) const;
};