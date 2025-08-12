#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClayBuilder.generated.h"

class AVoxelWorld;
class UTextureDetector;

UENUM()
enum class ECursorActionType : uint8
{
    Sculpt,
    Erase,
    Debug
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class VORONOITERRAIN_API UClayBuilder : public UActorComponent
{
    GENERATED_BODY()

public:
    UClayBuilder();

    void ApplyCursorAction(const FVector& MouseWorldLocation, const FVector& MouseWorldDirection, ECursorActionType CursorAction);
    bool GetBrushWorldLocation(const FVector& MouseWorldLocation, const FVector& MouseWorldDirection, ECursorActionType CursorAction, FVector& OutBrushWorldLocation) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AVoxelWorld* VoxelWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Resources")
    bool EnoughVoxel = true;

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

    UFUNCTION(BlueprintCallable, Category = "Build Settings")
    void AdjustBuildDistance(float DeltaDistance);

    UFUNCTION(BlueprintCallable, Category = "Build Settings")
    float GetCurrentBuildDistance() const { return MaxBuildDistance; }
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 100.0f, ClampMax = 2000.0f))
    float MaxBuildDistance = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Settings", meta = (ClampMin = "200.0", ClampMax = "5000.0"))
    float MinBuildDistance = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Settings", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
    float MaxBuildDistanceLimit = 3000.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextureDetector* TextureDetector;

    UFUNCTION(BlueprintCallable, Category = "Texture Detection")
    int32 GetTextureIdAtCursor() const;

protected:
    virtual void BeginPlay() override;

private:
    // --- Brush location helpers --- //
    bool AdjustHitLocation(const FVector & HitLocation, const FVector& MouseDirection, const FVector& MouseLocation,
    float CurrentBrushRadius, float ShiftRatio, FVector& OutBrushLocation) const;
    bool GetSafeSculptPosition(const FVector& MouseDirection, const FVector& MouseLocation, FVector& OutBrushLocation) const;

    // --- Voxel resource tracking helpers --- //
    float EstimateVoxelVolume(float BrushRadius) const;
    bool CanAffordVoxelOperation(float VolumeEstimate) const;
    void ConsumeVoxelAmount(float Amount);
    void AddVoxelAmount(float Amount);
};