#include "ClayBuilder.h"
#include "Components/DynamicMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UClayBuilder::UClayBuilder()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UClayBuilder::BeginPlay()
{
    Super::BeginPlay();

    if (!VoxelWorld)
    {
        VoxelWorld = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));
    }
}

//void UClayBuilder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//    FVector MouseWorldPosition;
//    if (GetMouseWorldPosition(MouseWorldPosition, 1.0f)) // Use positive strength for positioning
//    {
//        float CurrentBrushRadius = VoxelWorld ? VoxelWorld->BrushRadius : 90.0f;
//        DrawDebugSphere(GetWorld(), MouseWorldPosition, CurrentBrushRadius, 12, FColor::White, false, -1, 0, 2.0f);
//    }
//}

void UClayBuilder::StartBuildClay(ECursorActionType CursorAction)
{
    FVector MousePosition;
    if (!GetMouseWorldPosition(MousePosition, CursorAction) || !VoxelWorld)
        return;

    float CurrentBrushRadius = VoxelWorld ? VoxelWorld->BrushRadius : 90.0f;
    if (CursorAction == ECursorActionType::Erase) // Erasing
    {
        CurrentBrushRadius = VoxelWorld->EraseBrushRadius;
    }
    
    float VolumeEstimate = EstimateVoxelVolume(CurrentBrushRadius);
    
    // Check voxel amount
    if (CursorAction == ECursorActionType::Sculpt && !CanAffordVoxelOperation(VolumeEstimate))
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough voxel material!"));
        return;
    }
    float BrushStrength;
    switch (CursorAction)
    {
    case ECursorActionType::Erase:
        BrushStrength = -1.0f;
        break;
    case ECursorActionType::Sculpt:
        BrushStrength = 1.0f;
        break;
    default:
        BrushStrength = 0.0f;
        break;
    }
    // sculpting
    // UE_LOG(LogTemp, Warning, TEXT("Sculpt with %.2f"), BrushStrength);
    VoxelWorld->SculptAtPosition(MousePosition, BrushStrength, CurrentPhysicsType);
    
    // Update voxel amount
    if (CursorAction == ECursorActionType::Sculpt) // Drawing - consume
    {
        ConsumeVoxelAmount(VolumeEstimate);
    }
    else // Erasing - add back
    {
        AddVoxelAmount(VolumeEstimate);
    }
}

bool UClayBuilder::GetMouseWorldPosition(FVector& MouseWorldPosition, ECursorActionType CursorAction) const
{
    //FlushPersistentDebugLines(GetWorld());

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        return false;
    }

    FVector WorldLocation, WorldDirection;
    PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

    FHitResult HitResult;
    FVector TraceEnd = WorldLocation + (WorldDirection * MaxBuildDistance);
    if (CursorAction == ECursorActionType::Erase) {
        TraceEnd = WorldLocation + (WorldDirection * 10000);
    }

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    //QueryParams.AddIgnoredActor(VoxelWorld);
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = false;

    float CurrentBrushRadius = VoxelWorld ? VoxelWorld->BrushRadius : 90.0f;

    if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        // Check if hit a voxel mesh component
        if (UDynamicMeshComponent* HitMesh = Cast<UDynamicMeshComponent>(HitResult.GetComponent()))
        {
            switch (CursorAction)
            {
            case ECursorActionType::Erase:
                return GetCloserPositionIfHit(HitResult.Location, WorldDirection, WorldLocation, CurrentBrushRadius, -1, MouseWorldPosition);

            case ECursorActionType::Debug:
                return GetCloserPositionIfHit(HitResult.Location, WorldDirection, WorldLocation, CurrentBrushRadius, -0.5, MouseWorldPosition);

            case ECursorActionType::Sculpt:
                return GetSafeDrawPosition(MouseWorldPosition, WorldDirection, WorldLocation);
                // return GetCloserPositionIfHit(HitResult.Location, WorldDirection, WorldLocation, CurrentBrushRadius, 0, MouseWorldPosition);

            default:
                return false;
            }
            //UE_LOG(LogTemp, Warning, TEXT("/// A /// Hit Voxel: %s, Owner: %s, At: (%.2f, %.2f, %.2f) -0.5"),
            //    *HitResult.GetComponent()->GetName(),
            //    *HitResult.GetComponent()->GetOwner()->GetName(),
            //    HitResult.Location.X, HitResult.Location.Y, HitResult.Location.Z);
            //UE_LOG(LogTemp, Warning, TEXT("\t\t World Direction (%.2f, %.2f, %.2f)"),
            //    WorldDirection.X, WorldDirection.Y, WorldDirection.Z);
            //UE_LOG(LogTemp, Warning, TEXT("\t\t World Location (%.2f, %.2f, %.2f)"),
            //    WorldLocation.X, WorldLocation.Y, WorldLocation.Z);
            //return GetCloserPositionIfHit(HitResult.Location, WorldDirection, WorldLocation, CurrentBrushRadius, -1, MouseWorldPosition);

        }
        // Other mesh in ECC_WorldStatic channel
        //UE_LOG(LogTemp, Warning, TEXT("/// B /// Hit Other component in ECC_WorldStatic channel: %s, Owner: %s 0.5"),
        //    *HitResult.GetComponent()->GetName(),
        //    *HitResult.GetComponent()->GetOwner()->GetName());
        return GetCloserPositionIfHit(HitResult.Location, WorldDirection, WorldLocation, CurrentBrushRadius, 0.5, MouseWorldPosition);
    }

    if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_WorldDynamic, QueryParams))
    {
        //UE_LOG(LogTemp, Warning, TEXT("/// C /// Hit component in ECC_WorldDynamic channel: %s, Owner: %s 0.5"),
        //    *HitResult.GetComponent()->GetName(),
        //    *HitResult.GetComponent()->GetOwner()->GetName());
        return GetCloserPositionIfHit(HitResult.Location, WorldDirection, WorldLocation, CurrentBrushRadius, 0.5, MouseWorldPosition);
    }
    switch (CursorAction)
    {
    case ECursorActionType::Erase:
        return false;

    default:
        //UE_LOG(LogTemp, Warning, TEXT("/// D /// Draw on sky"));
        MouseWorldPosition = WorldLocation + (WorldDirection * MaxBuildDistance);
        if (CursorAction == ECursorActionType::Debug)
        {
            return true;
        }
        if (CursorAction == ECursorActionType::Erase) {
            MouseWorldPosition = WorldLocation + (WorldDirection * 10000);
            return true;
        }
        return GetSafeDrawPosition(MouseWorldPosition, WorldDirection, WorldLocation);
    }

}

bool UClayBuilder::GetCloserPositionIfHit(const FVector & HitLocation, const FVector& WorldDirection, const FVector& WorldLocation,
    float CurrentBrushRadius, float GapSize, FVector& MouseWorldPosition) const
{
    FVector CloserPosition = HitLocation - (WorldDirection * (CurrentBrushRadius * GapSize));
    if (FVector::Dist(CloserPosition, GetOwner()->GetActorLocation()) > 200.0f && FVector::Dist(CloserPosition, WorldLocation) > 200.0f)
    {
        MouseWorldPosition = CloserPosition;
        //UE_LOG(LogTemp, Warning, TEXT("ClayBuilder: Hit a mesh component at(%.2f, %.2f, %.2f)"),
        //    MouseWorldPosition.X, MouseWorldPosition.Y, MouseWorldPosition.Z);
        return true;
    }
    return false;
}

bool UClayBuilder::GetSafeDrawPosition(FVector& MouseWorldPosition, const FVector& WorldDirection, const FVector& WorldLocation) const
{
    FVector SafePosition = WorldLocation + (WorldDirection * MaxBuildDistance);
    if (FVector::Dist(SafePosition, GetOwner()->GetActorLocation()) > 50.0f && FVector::Dist(SafePosition, WorldLocation) > 200.0f)
    {
        MouseWorldPosition = SafePosition;
        return true;
    }
    return false;
}

float UClayBuilder::EstimateVoxelVolume(float BrushRadius) const
{
    // Estimate volume as sphere: (4/3) * π * r³
    // Scale it down to reasonable consumption rate
    float Volume = (4.0f / 3.0f) * PI * FMath::Pow(BrushRadius, 3);
    return Volume * VoxelConsumptionRate * 0.001f; // Scale factor for reasonable consumption
}

bool UClayBuilder::CanAffordVoxelOperation(float VolumeEstimate) const
{
    return CurrentVoxelAmount >= VolumeEstimate;
}

void UClayBuilder::ConsumeVoxelAmount(float Amount)
{
    CurrentVoxelAmount = FMath::Max(0.0f, CurrentVoxelAmount - Amount);
}

void UClayBuilder::AddVoxelAmount(float Amount)
{
    CurrentVoxelAmount = FMath::Min(MaxVoxelAmount, CurrentVoxelAmount + Amount);
}

void UClayBuilder::SetCurrentPhysicsType(EVoxelPhysicsType NewType)
{
    CurrentPhysicsType = NewType;
}

void UClayBuilder::AdjustBuildDistance(float DeltaDistance)
{
    MaxBuildDistance = FMath::Clamp(MaxBuildDistance + DeltaDistance, MinBuildDistance, MaxBuildDistanceLimit);

    // UE_LOG(LogTemp, Log, TEXT("Build Distance adjusted to: %.1f"), MaxBuildDistance);
}