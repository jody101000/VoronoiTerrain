#include "PlatformTemplate.h"

FBox FPlatformTemplate::GetLocalBounds() const
{
    if (Platforms.Num() == 0)
    {
        return FBox(EForceInit::ForceInit);
    }

    FBox Bounds(EForceInit::ForceInit);
    for (const FPlatformPlacement& Platform : Platforms)
    {
        Bounds += Platform.RelativePosition;
    }

    return Bounds;
}

bool FPlatformTemplate::IsValidTemplate() const
{
    // Check if we have at least entry and exit platforms
    if (Platforms.Num() < 2)
    {
        return false;
    }

    // Validate indices
    if (!Platforms.IsValidIndex(EntryPointIndex) || !Platforms.IsValidIndex(ExitPointIndex))
    {
        return false;
    }

    // Validate additional connection points
    for (int32 Index : AdditionalConnectionPoints)
    {
        if (!Platforms.IsValidIndex(Index))
        {
            return false;
        }
    }

    return true;
}

float FPlatformTemplate::GetTotalDistance() const
{
    if (!IsValidTemplate())
    {
        return 0.0f;
    }

    FVector EntryPos = Platforms[EntryPointIndex].RelativePosition;
    FVector ExitPos = Platforms[ExitPointIndex].RelativePosition;

    return FVector::Dist(EntryPos, ExitPos);
}