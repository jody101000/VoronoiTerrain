#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "VoxelData.h"
#include "VoxelBrush.h"
#include "FastNoiseLite.h"
#include "Components/BoxComponent.h"
#include "DynamicVoxelChunk.generated.h"

class UVoxelWorldManager;

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
    float VoxelSize = 20.0f;

    UPROPERTY(BlueprintReadWrite)
    UDynamicMeshComponent* MeshComponent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    UMaterialInstance* Material;

    // Chunk world position (in chunk coordinates)
    UPROPERTY(BlueprintReadOnly)
    FIntVector ChunkCoordinates;

    void Initialize(FIntVector InChunkCoordinates, float InChunkSize, float InVoxelSize);
    int32 Sculpt(UVoxelBrush* VoxelBrush);
    void UpdateMesh();
    bool IsEmpty() const;

    UPROPERTY()
    UVoxelWorldManager* WorldManager;

    UFUNCTION(BlueprintCallable)
    float GetTextureIdAtLocalPosition(const FVector& LocalPosition) const;

    UFUNCTION(BlueprintCallable)
    float GetTextureIdAtWorldPosition(const FVector& WorldPosition) const;

    FVoxel* VoxelData;

private:
    static FastNoiseLite Noise;
    FVector GetWorldPositionFromVoxelIndex(int X, int Y, int Z) const;
    FIntVector GetVoxelIndexFromWorldPosition(const FVector& WorldPos) const;
    FIntVector GetWorldVoxelCoordinates(int LocalX, int LocalY, int LocalZ) const;
};