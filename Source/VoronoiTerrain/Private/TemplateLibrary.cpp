#include "TemplateLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UTemplateLibrary::UTemplateLibrary()
{
    InitializeDefaultTemplates();
}

void UTemplateLibrary::InitializeDefaultTemplates()
{
    // Create default templates for each category

    // Easy Jump Templates
    RegisterTemplate(CreateSimpleJumpTemplate(200.0f, EPlatformType::Standard));
    RegisterTemplate(CreateSimpleJumpTemplate(250.0f, EPlatformType::Standard));

    // Medium Jump Templates
    RegisterTemplate(CreateSteppingStoneTemplate(400.0f, 2));
    RegisterTemplate(CreateSimpleJumpTemplate(350.0f, EPlatformType::Bounce));

    // Hard Jump Templates
    RegisterTemplate(CreateSteppingStoneTemplate(600.0f, 3));

    // Vertical Templates
    RegisterTemplate(CreateVerticalClimbTemplate(300.0f, 3));
    RegisterTemplate(CreateVerticalClimbTemplate(500.0f, 5));

    // Branch Templates
    RegisterTemplate(CreateBranchTemplate(30.0f));
    RegisterTemplate(CreateBranchTemplate(60.0f));
}

void UTemplateLibrary::RegisterTemplate(const FPlatformTemplate& Template)
{
    if (!Template.IsValidTemplate())
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempting to register invalid template"));
        return;
    }

    FTemplateEntry NewEntry;
    NewEntry.Template = Template;
    NewEntry.Category = Template.Metadata.Category;
    NewEntry.TemplateID = AllTemplates.Num();

    AllTemplates.Add(NewEntry);

    // Update category index
    RebuildCategoryIndices();
}

void UTemplateLibrary::RebuildCategoryIndices()
{
    TemplateCategoryIndices.Empty();

    // Build index strings for each category
    TMap<ETemplateCategory, TArray<int32>> TempIndices;

    for (int32 i = 0; i < AllTemplates.Num(); i++)
    {
        ETemplateCategory Category = AllTemplates[i].Category;
        if (!TempIndices.Contains(Category))
        {
            TempIndices.Add(Category, TArray<int32>());
        }
        TempIndices[Category].Add(i);
    }

    // Convert to comma-separated strings
    for (const auto& Pair : TempIndices)
    {
        FString IndexString;
        for (int32 i = 0; i < Pair.Value.Num(); i++)
        {
            if (i > 0)
            {
                IndexString += TEXT(",");
            }
            IndexString += FString::FromInt(Pair.Value[i]);
        }
        TemplateCategoryIndices.Add(Pair.Key, IndexString);
    }
}

TArray<int32> UTemplateLibrary::GetIndicesForCategory(ETemplateCategory Category) const
{
    TArray<int32> Indices;

    if (const FString* IndexString = TemplateCategoryIndices.Find(Category))
    {
        TArray<FString> IndexStrings;
        IndexString->ParseIntoArray(IndexStrings, TEXT(","), true);

        for (const FString& Str : IndexStrings)
        {
            Indices.Add(FCString::Atoi(*Str));
        }
    }

    return Indices;
}

TArray<FPlatformTemplate> UTemplateLibrary::GetTemplatesByCategory(ETemplateCategory Category) const
{
    TArray<FPlatformTemplate> Templates;
    TArray<int32> Indices = GetIndicesForCategory(Category);

    for (int32 Index : Indices)
    {
        if (AllTemplates.IsValidIndex(Index))
        {
            Templates.Add(AllTemplates[Index].Template);
        }
    }

    return Templates;
}

FPlatformTemplate UTemplateLibrary::SelectTemplate(const FTemplateSelectionCriteria& Criteria, int32 RandomSeed) const
{
    TArray<int32> CandidateIndices;
    TArray<float> Scores;

    // Gather all candidate templates
    if (Criteria.PreferredCategories.Num() > 0)
    {
        // Use preferred categories
        for (ETemplateCategory Category : Criteria.PreferredCategories)
        {
            TArray<int32> CategoryIndicesArray = GetIndicesForCategory(Category);
            for (int32 Index : CategoryIndicesArray)
            {
                if (AllTemplates.IsValidIndex(Index) &&
                    AllTemplates[Index].Template.Metadata.DifficultyRating <= Criteria.MaxDifficulty)
                {
                    CandidateIndices.Add(Index);
                    Scores.Add(CalculateTemplateScore(AllTemplates[Index].Template, Criteria));
                }
            }
        }
    }
    else
    {
        // Consider all templates
        for (int32 i = 0; i < AllTemplates.Num(); i++)
        {
            if (AllTemplates[i].Template.Metadata.DifficultyRating <= Criteria.MaxDifficulty)
            {
                CandidateIndices.Add(i);
                Scores.Add(CalculateTemplateScore(AllTemplates[i].Template, Criteria));
            }
        }
    }

    if (CandidateIndices.Num() == 0)
    {
        // Return a default simple jump template as fallback
        return CreateSimpleJumpTemplate(Criteria.RequiredHorizontalDistance);
    }

    // Select template based on scores with some randomness
    FRandomStream RandomStream(RandomSeed);

    // Find best score
    float BestScore = 0.0f;
    for (float Score : Scores)
    {
        BestScore = FMath::Max(BestScore, Score);
    }

    // Select from templates with scores close to best
    TArray<int32> GoodCandidates;
    for (int32 i = 0; i < Scores.Num(); i++)
    {
        if (Scores[i] >= BestScore * 0.8f) // Within 80% of best score
        {
            GoodCandidates.Add(i);
        }
    }

    int32 SelectedCandidateIndex = GoodCandidates[RandomStream.RandRange(0, GoodCandidates.Num() - 1)];
    int32 TemplateIndex = CandidateIndices[SelectedCandidateIndex];

    return AllTemplates[TemplateIndex].Template;
}

// Rest of the implementation remains the same...
float UTemplateLibrary::CalculateTemplateScore(const FPlatformTemplate& Template, const FTemplateSelectionCriteria& Criteria) const
{
    float Score = 100.0f;

    // Distance matching score
    float HorizontalDiff = FMath::Abs(Template.Metadata.RequiredHorizontalDistance - Criteria.RequiredHorizontalDistance);
    float VerticalDiff = FMath::Abs(Template.Metadata.RequiredVerticalDistance - Criteria.RequiredVerticalDistance);

    Score -= (HorizontalDiff / Criteria.RequiredHorizontalDistance) * 50.0f;
    Score -= (VerticalDiff / FMath::Max(1.0f, Criteria.RequiredVerticalDistance)) * 30.0f;

    // Deformation capability bonus
    if (Template.Metadata.bSupportsDeformation)
    {
        float DeformRange = Criteria.RequiredHorizontalDistance / Template.Metadata.RequiredHorizontalDistance;
        if (DeformRange >= Template.Metadata.MinDeformScale && DeformRange <= Template.Metadata.MaxDeformScale)
        {
            Score += 20.0f;
        }
    }

    // Difficulty appropriateness
    float DifficultyRatio = (float)Template.Metadata.DifficultyRating / (float)Criteria.MaxDifficulty;
    Score += (1.0f - FMath::Abs(DifficultyRatio - 0.7f)) * 10.0f; // Prefer 70% of max difficulty

    return FMath::Max(0.0f, Score);
}

FPlatformTemplate UTemplateLibrary::CreateSimpleJumpTemplate(float GapDistance, EPlatformType PlatformType)
{
    FPlatformTemplate Template;

    // Metadata
    Template.Metadata.TemplateName = FString::Printf(TEXT("Simple Jump %.0f"), GapDistance);
    Template.Metadata.Category = GapDistance < 300.0f ? ETemplateCategory::EasyJump : ETemplateCategory::MediumJump;
    Template.Metadata.RequiredHorizontalDistance = GapDistance;
    Template.Metadata.RequiredVerticalDistance = 0.0f;
    Template.Metadata.DifficultyRating = FMath::Clamp(FMath::RoundToInt(GapDistance / 50.0f), 1, 10);
    Template.Metadata.bSupportsDeformation = true;

    // Platforms
    FPlatformPlacement Entry;
    Entry.RelativePosition = FVector::ZeroVector;
    Entry.PlatformType = PlatformType;
    Entry.ConnectionPointIndex = 0;

    FPlatformPlacement Exit;
    Exit.RelativePosition = FVector(GapDistance, 0.0f, 0.0f);
    Exit.PlatformType = PlatformType;
    Exit.ConnectionPointIndex = 1;

    Template.Platforms.Add(Entry);
    Template.Platforms.Add(Exit);
    Template.EntryPointIndex = 0;
    Template.ExitPointIndex = 1;

    return Template;
}

FPlatformTemplate UTemplateLibrary::CreateSteppingStoneTemplate(float TotalDistance, int32 StoneCount)
{
    FPlatformTemplate Template;

    // Metadata
    Template.Metadata.TemplateName = FString::Printf(TEXT("Stepping Stones %d"), StoneCount);
    Template.Metadata.Category = StoneCount <= 2 ? ETemplateCategory::MediumJump : ETemplateCategory::HardJump;
    Template.Metadata.RequiredHorizontalDistance = TotalDistance;
    Template.Metadata.RequiredVerticalDistance = 0.0f;
    Template.Metadata.DifficultyRating = FMath::Clamp(3 + StoneCount * 2, 1, 10);
    Template.Metadata.bSupportsDeformation = true;

    // Create platforms
    float StepDistance = TotalDistance / (StoneCount + 1);

    for (int32 i = 0; i <= StoneCount + 1; i++)
    {
        FPlatformPlacement Platform;
        Platform.RelativePosition = FVector(i * StepDistance, 0.0f, 0.0f);

        // Vary platform types for middle stones
        if (i > 0 && i <= StoneCount)
        {
            Platform.PlatformType = (i % 2 == 0) ? EPlatformType::Standard : EPlatformType::Rotating;
            Platform.Scale = 0.8f; // Smaller stepping stones
        }
        else
        {
            Platform.PlatformType = EPlatformType::Standard;
        }

        Platform.ConnectionPointIndex = (i == 0) ? 0 : (i == StoneCount + 1) ? 1 : -1;
        Template.Platforms.Add(Platform);
    }

    Template.EntryPointIndex = 0;
    Template.ExitPointIndex = StoneCount + 1;

    return Template;
}

FPlatformTemplate UTemplateLibrary::CreateVerticalClimbTemplate(float Height, int32 PlatformCount)
{
    FPlatformTemplate Template;

    // Metadata
    Template.Metadata.TemplateName = FString::Printf(TEXT("Vertical Climb %.0f"), Height);
    Template.Metadata.Category = ETemplateCategory::Vertical;
    Template.Metadata.RequiredHorizontalDistance = 100.0f;
    Template.Metadata.RequiredVerticalDistance = Height;
    Template.Metadata.DifficultyRating = FMath::Clamp(FMath::RoundToInt(Height / 100.0f) + 2, 1, 10);
    Template.Metadata.MinimumClearanceHeight = Height + 200.0f;

    // Create spiral climbing pattern
    float VerticalStep = Height / (PlatformCount - 1);
    float AngleStep = 360.0f / PlatformCount;
    float Radius = 150.0f;

    for (int32 i = 0; i < PlatformCount; i++)
    {
        FPlatformPlacement Platform;
        float Angle = FMath::DegreesToRadians(i * AngleStep);

        Platform.RelativePosition = FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            i * VerticalStep
        );

        Platform.PlatformType = (i % 3 == 0) ? EPlatformType::Bounce : EPlatformType::Standard;
        Platform.ConnectionPointIndex = (i == 0) ? 0 : (i == PlatformCount - 1) ? 1 : -1;

        Template.Platforms.Add(Platform);
    }

    Template.EntryPointIndex = 0;
    Template.ExitPointIndex = PlatformCount - 1;

    return Template;
}

FPlatformTemplate UTemplateLibrary::CreateBranchTemplate(float BranchAngle)
{
    FPlatformTemplate Template;

    // Metadata
    Template.Metadata.TemplateName = FString::Printf(TEXT("Branch %.0fdeg"), BranchAngle);
    Template.Metadata.Category = ETemplateCategory::Branch;
    Template.Metadata.RequiredHorizontalDistance = 300.0f;
    Template.Metadata.DifficultyRating = 5;

    // Create Y-shaped branch
    FPlatformPlacement Entry;
    Entry.RelativePosition = FVector::ZeroVector;
    Entry.PlatformType = EPlatformType::Standard;
    Entry.ConnectionPointIndex = 0;

    FPlatformPlacement Center;
    Center.RelativePosition = FVector(150.0f, 0.0f, 0.0f);
    Center.PlatformType = EPlatformType::Standard;

    FPlatformPlacement LeftExit;
    float LeftAngleRad = FMath::DegreesToRadians(-BranchAngle);
    LeftExit.RelativePosition = Center.RelativePosition + FVector(
        FMath::Cos(LeftAngleRad) * 150.0f,
        FMath::Sin(LeftAngleRad) * 150.0f,
        50.0f
    );
    LeftExit.PlatformType = EPlatformType::Bounce;
    LeftExit.ConnectionPointIndex = 1;

    FPlatformPlacement RightExit;
    float RightAngleRad = FMath::DegreesToRadians(BranchAngle);
    RightExit.RelativePosition = Center.RelativePosition + FVector(
        FMath::Cos(RightAngleRad) * 150.0f,
        FMath::Sin(RightAngleRad) * 150.0f,
        0.0f
    );
    RightExit.PlatformType = EPlatformType::Standard;
    RightExit.ConnectionPointIndex = 2;

    Template.Platforms.Add(Entry);
    Template.Platforms.Add(Center);
    Template.Platforms.Add(LeftExit);
    Template.Platforms.Add(RightExit);

    Template.EntryPointIndex = 0;
    Template.ExitPointIndex = 2; // Primary exit
    Template.AdditionalConnectionPoints.Add(3); // Secondary exit

    return Template;
}

TArray<FVector> UTemplateLibrary::TransformTemplateToWorld(
    const FPlatformTemplate& Template,
    const FVector& EntryWorldPos,
    const FVector& ExitWorldPos,
    float DeformScale)
{
    TArray<FVector> WorldPositions;

    if (!Template.IsValidTemplate())
    {
        return WorldPositions;
    }

    // Get template entry and exit positions
    FVector TemplateEntry = Template.Platforms[Template.EntryPointIndex].RelativePosition;
    FVector TemplateExit = Template.Platforms[Template.ExitPointIndex].RelativePosition;

    // Calculate required transformation
    FVector TemplateDir = (TemplateExit - TemplateEntry).GetSafeNormal();
    FVector WorldDir = (ExitWorldPos - EntryWorldPos).GetSafeNormal();

    // Calculate rotation
    FQuat Rotation = FQuat::FindBetweenNormals(TemplateDir, WorldDir);

    // Calculate scale
    float TemplateDistance = FVector::Dist(TemplateEntry, TemplateExit);
    float WorldDistance = FVector::Dist(EntryWorldPos, ExitWorldPos);
    float Scale = (WorldDistance / TemplateDistance) * DeformScale;

    // Apply transformation to all platforms
    for (const FPlatformPlacement& Platform : Template.Platforms)
    {
        // Translate to origin
        FVector LocalPos = Platform.RelativePosition - TemplateEntry;

        // Apply scale
        LocalPos *= Scale;

        // Apply rotation
        LocalPos = Rotation.RotateVector(LocalPos);

        // Translate to world position
        FVector WorldPos = EntryWorldPos + LocalPos;

        WorldPositions.Add(WorldPos);
    }

    return WorldPositions;
}