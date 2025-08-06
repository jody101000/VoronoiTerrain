#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorld.h"
#include "ClayBuilder.generated.h"

UENUM()
enum ECursorActionType
{
    Sculpt  UMETA(DisplayName = "Sculpting"),
    Erase   UMETA(DisplayName = "Erasing"),
    Debug   UMETA(DisplayName = "Debugging"),
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class VORONOITERRAIN_API UClayBuilder : public UActorComponent
{
    GENERATED_BODY()

public:
    UClayBuilder();

    //virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void StartBuildClay(ECursorActionType CursorAction);

    bool GetMouseWorldPosition(FVector& MouseWorldPosition, ECursorActionType CursorAction) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AVoxelWorld* VoxelWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Resources")
    float CurrentVoxelAmount = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Resources") 
    float MaxVoxelAmount = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Resources")
    float VoxelConsumptionRate = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Voxel Resources")
    float GetVoxelAmount() const { return CurrentVoxelAmount; }

    UFUNCTION(BlueprintCallable, Category = "Voxel Resources")
    float GetMaxVoxelAmount() const { return MaxVoxelAmount; }

    UFUNCTION(BlueprintCallable, Category = "Voxel Resources")
    float GetVoxelAmountPercentage() const { return CurrentVoxelAmount / MaxVoxelAmount; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 100.0f, ClampMax = 2000.0f))
    float MaxBuildDistance = 1000.0f;

    bool GetCloserPositionIfHit(const FVector& HitLocation, const FVector& WorldDirection, const FVector& WorldLocation,
        float CurrentBrushRadius, float GapSize, FVector& MouseWorldPosition) const;

private:
    float EstimateVoxelVolume(float BrushRadius) const;
    bool CanAffordVoxelOperation(float VolumeEstimate) const;
    void ConsumeVoxelAmount(float Amount);
    void AddVoxelAmount(float Amount);

};