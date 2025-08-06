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

void UClayBuilder::StartBuildClay(float BrushStrength)
{
    FVector MousePosition;
    if (GetMouseWorldPosition(MousePosition, BrushStrength) && VoxelWorld)
    {
        VoxelWorld->SculptAtPosition(MousePosition, BrushStrength);
    }
}

bool UClayBuilder::GetMouseWorldPosition(FVector& MouseWorldPosition, float BrushStrength) const
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
            if (BrushStrength == -1) 
            {
                return GetCloserPositionIfHit(HitResult.Location, WorldDirection, WorldLocation, CurrentBrushRadius, -0.5, MouseWorldPosition);
            }
            if (BrushStrength == 0.5)
            {
                return GetCloserPositionIfHit(HitResult.Location, WorldDirection, WorldLocation, CurrentBrushRadius, -1, MouseWorldPosition);
            }
            return false;
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
    
    //UE_LOG(LogTemp, Warning, TEXT("/// D /// Draw on sky"));
    MouseWorldPosition = WorldLocation + (WorldDirection * MaxBuildDistance);
    return true;
}

bool UClayBuilder::GetCloserPositionIfHit(const FVector & HitLocation, const FVector& WorldDirection, const FVector& WorldLocation,
    float CurrentBrushRadius, float GapSize, FVector& MouseWorldPosition) const
{
    FVector CloserPosition = HitLocation - (WorldDirection * (CurrentBrushRadius * GapSize));
    if (FVector::Dist(CloserPosition, WorldLocation) > 200.0f)
    {
        MouseWorldPosition = CloserPosition;
        //UE_LOG(LogTemp, Warning, TEXT("ClayBuilder: Hit a mesh component at(%.2f, %.2f, %.2f)"),
        //    MouseWorldPosition.X, MouseWorldPosition.Y, MouseWorldPosition.Z);
        return true;
    }
    return false;
}