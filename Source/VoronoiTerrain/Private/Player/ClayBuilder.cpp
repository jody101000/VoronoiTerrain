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

void UClayBuilder::ApplyCursorAction(const FVector& MouseWorldLocation, const FVector& MouseWorldDirection, ECursorActionType CursorAction)
{
    FVector BrushWorldLocation;
    if (!GetBrushWorldLocation(MouseWorldLocation, MouseWorldDirection, CursorAction, BrushWorldLocation) || !VoxelWorld)
    {
        return;
    }

    float CurrentBrushRadius = VoxelWorld ? VoxelWorld->BrushRadius : 90.0f;
    if (CursorAction == ECursorActionType::Erase)
    {
        CurrentBrushRadius = VoxelWorld->EraseBrushRadius;
    }

    // No sculpt if overlap with palyer or camera
    APawn* Player = GetWorld()->GetFirstPlayerController()->GetPawn();
    AActor* Camera = GetWorld()->GetFirstPlayerController()->GetViewTarget();
    bool bPlayerOverlap = Player && FVector::Dist(Player->GetActorLocation(), BrushWorldLocation) < CurrentBrushRadius;
    bool bCameraOverlap = Camera && FVector::Dist(Camera->GetActorLocation(), BrushWorldLocation) < CurrentBrushRadius;
    if (bPlayerOverlap || bCameraOverlap) {
        return;
    }

    // No sculpt if voxel amount not enough
    float ConsumptionEstimate = EstimateVoxelVolume(CurrentBrushRadius);
    if (CursorAction == ECursorActionType::Sculpt && !CanAffordVoxelOperation(ConsumptionEstimate))
    {
        EnoughVoxel = false;
        return;
    }
    EnoughVoxel = true;

    // Setup brush strength
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
        return;
    }
    
    // Sculpt
    int32 ActualVoxelChanges = VoxelWorld->SculptAtPosition(BrushWorldLocation, BrushStrength);
    
    float VoxelSize = VoxelWorld->VoxelSize;
    float VoxelVolume = VoxelSize * VoxelSize * VoxelSize;
    float ActualConsumption = ActualVoxelChanges * VoxelVolume * VoxelConsumptionRate * 0.001f;

    // Update voxel amount
    if (CursorAction == ECursorActionType::Sculpt)
    {
        ConsumeVoxelAmount(ActualConsumption);
    }
    else
    {
        AddVoxelAmount(ActualConsumption);
    }
}

/**
 * Line trace mouse to get a brush location
 * @param MouseLocation 3D mouse location in world
 * @param MouseDirection 3D mouse direction in world
 * @param CursorAction Sculpt, Erase, Debug
 * @param OutBrushLocation Output. Will be set to the brush location in world
 * @return true if a proper brush position is found; false otherwise
 */
bool UClayBuilder::GetBrushWorldLocation(const FVector& MouseLocation, const FVector& MouseDirection, ECursorActionType CursorAction, FVector& OutBrushLocation) const
{
    // Line trace setups
    FHitResult HitResult;
    FVector TraceEnd = MouseLocation + (MouseDirection * MaxBuildDistance);
    if (CursorAction == ECursorActionType::Erase) { // Trace further for erasing
        TraceEnd = MouseLocation + (MouseDirection * 10000);
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = false;

    float CurrentBrushRadius = VoxelWorld ? VoxelWorld->BrushRadius : 90.0f;
    float ShiftRatio = 0.5; // Default: half radius out from hit object
    
    // Check hit ECC_WorldStatic
    if (GetWorld()->LineTraceSingleByChannel(HitResult, MouseLocation, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        // Check hit voxel
        if (UDynamicMeshComponent* HitMesh = Cast<UDynamicMeshComponent>(HitResult.GetComponent()))
        {
            switch (CursorAction)
            {
            case ECursorActionType::Sculpt:     // Distance set by mouse scroll
                return GetSafeSculptPosition(MouseDirection, MouseLocation, OutBrushLocation);
            case ECursorActionType::Erase:      // Full radius into hit object
                ShiftRatio = -1;
                break;
            case ECursorActionType::Debug:      // Half radius into hit object
                ShiftRatio = -0.5;
                break;
            default:
                return false;
            }
        }
        // Half radius out from hit object
        return AdjustHitLocation(HitResult.Location, MouseDirection, MouseLocation, CurrentBrushRadius, ShiftRatio, OutBrushLocation);
    }

    // Check hit ECC_WorldDynamic
    if (GetWorld()->LineTraceSingleByChannel(HitResult, MouseLocation, TraceEnd, ECC_WorldDynamic, QueryParams))
    {
        switch (CursorAction)
        {
        case ECursorActionType::Erase:  // Do not erase
            return false;
        default:                        // Half radius out from hit object
            return AdjustHitLocation(HitResult.Location, MouseDirection, MouseLocation, CurrentBrushRadius, ShiftRatio, OutBrushLocation);
        }
    }

    // Hit other or hit nothing
    switch (CursorAction)
    {
    case ECursorActionType::Erase:      // Do not erase
        return false;
    default:                            // Distance set by mouse scroll
        return GetSafeSculptPosition(MouseDirection, MouseLocation, OutBrushLocation);
    }
}

void UClayBuilder::AdjustBuildDistance(float DeltaDistance)
{
    MaxBuildDistance = FMath::Clamp(MaxBuildDistance + DeltaDistance, MinBuildDistance, MaxBuildDistanceLimit);
}

bool UClayBuilder::AdjustHitLocation(const FVector & HitLocation, const FVector& MouseDirection, const FVector& MouseLocation,
    float CurrentBrushRadius, float ShiftRatio, FVector& OutBrushLocation) const
{
    FVector AdjustedLocation = HitLocation - (MouseDirection * (CurrentBrushRadius * ShiftRatio));
    if (FVector::Dist(AdjustedLocation, GetOwner()->GetActorLocation()) > MinBuildDistance &&
        FVector::Dist(AdjustedLocation, MouseLocation) > MinBuildDistance)
    {
        OutBrushLocation = AdjustedLocation;
        return true;
    }
    return false;
}

bool UClayBuilder::GetSafeSculptPosition(const FVector& MouseDirection, const FVector& MouseLocation, FVector& OutBrushLocation) const
{
    FVector SafePosition = MouseLocation + (MouseDirection * MaxBuildDistance);
    if (FVector::Dist(SafePosition, GetOwner()->GetActorLocation() + FVector(0,0,60)) > MinBuildDistance &&
        FVector::Dist(SafePosition, MouseLocation) > MinBuildDistance)
    {
        OutBrushLocation = SafePosition;
        return true;
    }
    return false;
}

float UClayBuilder::EstimateVoxelVolume(float BrushRadius) const
{
    // Estimate volume as sphere
    float Volume = (4.0f / 3.0f) * PI * FMath::Pow(BrushRadius, 3);
    return Volume * VoxelConsumptionRate * 0.001f;
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

int32 UClayBuilder::GetTextureIdAtCursor() const
{
    if (TextureDetector)
    {
        return TextureDetector->DetectTextureAtMousePosition();
    }
    return 0;
}