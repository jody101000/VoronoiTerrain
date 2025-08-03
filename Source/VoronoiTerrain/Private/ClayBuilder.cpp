#include "ClayBuilder.h"

UClayBuilder::UClayBuilder()
{
    PrimaryComponentTick.bCanEverTick = false;
    VoxelWorldManager = CreateDefaultSubobject<UVoxelWorldManager>("VoxelWorldManager");
}

void UClayBuilder::BeginPlay()
{
    Super::BeginPlay();

    if (VoxelWorldManager)
    {
        VoxelWorldManager->ChunkMaterial = VoxelMaterial;
        VoxelWorldManager->BrushRadius = BrushRadius;
        VoxelWorldManager->BrushStrength = BrushStrength;
    }
}

void UClayBuilder::StartBuildClay()
{
    FVector MousePosition;
    if (GetMouseWorldPosition(MousePosition) && VoxelWorldManager)
    {
        VoxelWorldManager->SculptAtPosition(MousePosition);
        UE_LOG(LogTemp, Warning, TEXT("Clay formed at (%.03f, %.03f, %.03f)"),
            MousePosition.X, MousePosition.Y, MousePosition.Z);
    }
}

bool UClayBuilder::GetMouseWorldPosition(FVector& MouseWorldPosition) const
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        return false;
    }

    float MouseX, MouseY;
    PC->GetMousePosition(MouseX, MouseY);

    FVector WorldLocation, WorldDirection;
    PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);

    FHitResult HitResult;
    FVector TraceEnd = WorldLocation + (WorldDirection * MaxBuildDistance);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        // Build slightly above the hit surface
        float SurfaceOffset = BrushRadius * 0.5f;
        MouseWorldPosition = HitResult.Location + (HitResult.Normal * SurfaceOffset);
    }
    else
    {
        // No surface hit, build at max distance
        MouseWorldPosition = WorldLocation + (WorldDirection * MaxBuildDistance);
    }

    return true;
}