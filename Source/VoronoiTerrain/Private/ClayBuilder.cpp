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
        UE_LOG(LogTemp, Warning, TEXT("ClayBuilder: Detect Mouse Hit (%.2f, %.2f, %.2f)"),
            MousePosition.X, MousePosition.Y, MousePosition.Z);
        VoxelWorldManager->SculptAtPosition(MousePosition);
    }
}

bool UClayBuilder::GetMouseWorldPosition(FVector& MouseWorldPosition) const
{
    //FlushPersistentDebugLines(GetWorld());

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
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = false;

    if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        // Check if hit a voxel mesh component
        if (UDynamicMeshComponent* HitMesh = Cast<UDynamicMeshComponent>(HitResult.GetComponent()))
        {
            MouseWorldPosition = HitResult.Location - (WorldDirection * (BrushRadius * 0.5f));
            UE_LOG(LogTemp, Warning, TEXT("ClayBuilder: Hit a voxel mesh component at(%.2f, %.2f, %.2f)"),
                MouseWorldPosition.X, MouseWorldPosition.Y, MouseWorldPosition.Z);
            DrawDebugLine(GetWorld(), WorldLocation, MouseWorldPosition, FColor::Red, false, -1.0, 0, 2.0f);
            DrawDebugSphere(GetWorld(), MouseWorldPosition, 12.0, 12, FColor::Red, false, -1.0f, 0, 1.0f);
            return true;
        }
        MouseWorldPosition = HitResult.Location - (WorldDirection * (BrushRadius * 0.5f));
        DrawDebugLine(GetWorld(), WorldLocation, MouseWorldPosition, FColor::Green, false, -1.0, 0, 2.0f);
        DrawDebugSphere(GetWorld(), MouseWorldPosition, 12.0, 12, FColor::Green, false, -1.0f, 0, 1.0f);
        return true;
    }

    if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_WorldDynamic, QueryParams))
    {
        MouseWorldPosition = HitResult.Location - (WorldDirection * (BrushRadius * 0.5f));
        DrawDebugLine(GetWorld(), WorldLocation, MouseWorldPosition, FColor::Purple, false, -1.0, 0, 2.0f);
        DrawDebugSphere(GetWorld(), MouseWorldPosition, 12.0, 12, FColor::Purple, false, -1.0f, 0, 1.0f);
        return true;
    }
    
    MouseWorldPosition = WorldLocation + (WorldDirection * MaxBuildDistance);
    DrawDebugLine(GetWorld(), WorldLocation, MouseWorldPosition, FColor::Blue, false, -1.0, 0, 2.0f);
    DrawDebugSphere(GetWorld(), MouseWorldPosition, 12.0, 12, FColor::Blue, false, -1.0f, 0, 1.0f);
    return true;
}