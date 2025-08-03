#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "VoxelData.h"
#include "VoxelBrush.h"
#include "DynamicVoxelChunk.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VORONOITERRAIN_API UDynamicVoxelChunk : public USceneComponent
{
    GENERATED_BODY()

public:
    UDynamicVoxelChunk();

protected:
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

public:
    UPROPERTY(BlueprintReadWrite)
    int ChunkSize = 32;

    UPROPERTY(BlueprintReadWrite)
    float VoxelSize = 100.0f;

    UPROPERTY(BlueprintReadWrite)
    UDynamicMeshComponent* MeshComponent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    UMaterialInstance* Material;

    // Chunk world position (in chunk coordinates)
    UPROPERTY(BlueprintReadOnly)
    FIntVector ChunkCoordinates;

    void Initialize(FIntVector InChunkCoordinates, float InVoxelSize);
    void Sculpt(UVoxelBrush* VoxelBrush);
    void UpdateMesh();
    bool IsEmpty() const;

private:
    FVoxel* VoxelData;
    bool bNeedsUpdate;

    FVector GetWorldPositionFromVoxelIndex(int X, int Y, int Z) const;
    FIntVector GetVoxelIndexFromWorldPosition(const FVector& WorldPos) const;
};