#include "Player/TextureDetector.h"
#include "VoxelSystem/VoxelWorld.h"
#include "Components/DynamicMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UTextureDetector::UTextureDetector()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTextureDetector::BeginPlay()
{
    Super::BeginPlay();

    if (!VoxelWorld)
    {
        VoxelWorld = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));
    }
}

int32 UTextureDetector::DetectTextureAtMousePosition() const
{
    FVector WorldPosition;
    if (GetMouseWorldPositionOnVoxels(WorldPosition))
    {
        int32 TextureId = DetectTextureAtWorldPosition(WorldPosition);

        // Broadcast the event
        if (OnTextureDetected.IsBound())
        {
            OnTextureDetected.Broadcast(TextureId, WorldPosition);
        }

        return TextureId;
    }

    return 0; // No texture found
}

int32 UTextureDetector::DetectTextureAtWorldPosition(const FVector& WorldPosition) const
{
    if (!VoxelWorld)
        return 0;

    return VoxelWorld->GetTextureIdAtWorldPosition(WorldPosition);
}

bool UTextureDetector::GetMouseWorldPositionOnVoxels(FVector& OutWorldPosition) const
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
        return false;

    FVector WorldLocation, WorldDirection;
    PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

    FHitResult HitResult;
    FVector TraceEnd = WorldLocation + (WorldDirection * MaxTraceDistance);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = false;

    // Trace against voxel meshes
    if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        // Check if we hit a voxel mesh component
        if (UDynamicMeshComponent* HitMesh = Cast<UDynamicMeshComponent>(HitResult.GetComponent()))
        {
            OutWorldPosition = HitResult.Location;
            return true;
        }
    }

    return false;
}