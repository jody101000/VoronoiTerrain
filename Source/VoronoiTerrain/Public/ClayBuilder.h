#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorld.h"
#include "ClayBuilder.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class VORONOITERRAIN_API UClayBuilder : public UActorComponent
{
    GENERATED_BODY()

public:
    UClayBuilder();

    //virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void StartBuildClay(float BrushStrength);

    bool GetMouseWorldPosition(FVector& MouseWorldPosition, float BrushStrength) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AVoxelWorld* VoxelWorld;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 100.0f, ClampMax = 2000.0f))
    float MaxBuildDistance = 1000.0f;

    bool GetCloserPositionIfHit(const FVector& HitLocation, const FVector& WorldDirection, const FVector& WorldLocation,
        float CurrentBrushRadius, float GapSize, FVector& MouseWorldPosition) const;
};