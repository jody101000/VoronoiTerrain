#include "Player/ClayBuilder.h"
#include "VoxelSystem/VoxelWorld.h"
#include "Components/DynamicMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Player/TextureDetector.h"
#include "Kismet/GameplayStatics.h"

UClayBuilder::UClayBuilder()
{
    PrimaryComponentTick.bCanEverTick = false;
    TextureDetector = CreateDefaultSubobject<UTextureDetector>(TEXT("TextureDetector"));
}

void UClayBuilder::BeginPlay()
{
    Super::BeginPlay();

    if (!VoxelWorld)
    {
        VoxelWorld = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));
    }
    if (TextureDetector)
    {
        TextureDetector->VoxelWorld = VoxelWorld;
    }
}

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
        EnoughVoxel = false;
        return;
    }
    EnoughVoxel = true;

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

    FVector SphereCenter = MousePosition;
    float SphereRadius = CurrentBrushRadius;

    // Get specific player and camera references
    APawn* Player = GetWorld()->GetFirstPlayerController()->GetPawn();
    AActor* Camera = GetWorld()->GetFirstPlayerController()->GetViewTarget();

    bool bPlayerOverlap = Player && FVector::Dist(Player->GetActorLocation(), SphereCenter) < SphereRadius;
    bool bCameraOverlap = Camera && FVector::Dist(Camera->GetActorLocation(), SphereCenter) < SphereRadius;

    if (bPlayerOverlap || bCameraOverlap) {
        // Don't sculpt if player would be affected
        return;
    }

    // sculpting
    // UE_LOG(LogTemp, Warning, TEXT("Sculpt with %.2f"), BrushStrength);
    int32 ActualVoxelChanges = VoxelWorld->SculptAtPosition(MousePosition, BrushStrength);
    
    float VoxelSize = VoxelWorld->VoxelSize;
    float VoxelVolume = VoxelSize * VoxelSize * VoxelSize;
    float ActualConsumption = ActualVoxelChanges * VoxelVolume * VoxelConsumptionRate * 0.001f;

    // Update voxel amount
    if (CursorAction == ECursorActionType::Sculpt) // Drawing - consume
    {
        ConsumeVoxelAmount(ActualConsumption);
    }
    else // Erasing - add back
    {
        AddVoxelAmount(ActualConsumption);
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

        }
        return GetCloserPositionIfHit(HitResult.Location, WorldDirection, WorldLocation, CurrentBrushRadius, 0.5, MouseWorldPosition);
    }

    if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_WorldDynamic, QueryParams))
    {
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
    if (FVector::Dist(CloserPosition, GetOwner()->GetActorLocation()) > MinBuildDistance &&
        FVector::Dist(CloserPosition, WorldLocation) > MinBuildDistance)
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
    if (FVector::Dist(SafePosition, GetOwner()->GetActorLocation() + FVector(0,0,60)) > MinBuildDistance &&
        FVector::Dist(SafePosition, WorldLocation) > MinBuildDistance)
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

void UClayBuilder::AdjustBuildDistance(float DeltaDistance)
{
    MaxBuildDistance = FMath::Clamp(MaxBuildDistance + DeltaDistance, MinBuildDistance, MaxBuildDistanceLimit);

    // UE_LOG(LogTemp, Log, TEXT("Build Distance adjusted to: %.1f"), MaxBuildDistance);
}

int32 UClayBuilder::GetTextureIdAtCursor() const
{
    if (TextureDetector)
    {
        return TextureDetector->DetectTextureAtMousePosition();
    }
    return 0;
}