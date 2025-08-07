#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "VoxelData.h"
#include "VoxelBrush.h"
#include "FastNoiseLite.h"
#include "VoxelPhysicsTypes.h"
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
    float VoxelSize = 100.0f;

    UPROPERTY(BlueprintReadWrite)
    UDynamicMeshComponent* MeshComponent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    UMaterialInstance* Material;

    // Chunk world position (in chunk coordinates)
    UPROPERTY(BlueprintReadOnly)
    FIntVector ChunkCoordinates;

    UPROPERTY(BlueprintReadWrite)
    EVoxelPhysicsType CurrentPhysicsType = EVoxelPhysicsType::Standard;

    UPROPERTY(BlueprintReadWrite)
    FVoxelPhysicsProperties PhysicsProperties;

    UPROPERTY()
    UBoxComponent* PhysicsTriggerVolume;

    void SetPhysicsType(EVoxelPhysicsType NewType);

    UFUNCTION()
    void OnVoxelBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnVoxelEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void Initialize(FIntVector InChunkCoordinates, float InVoxelSize);
    void Sculpt(UVoxelBrush* VoxelBrush);
    void UpdateMesh();
    bool IsEmpty() const;

    UPROPERTY()
    UVoxelWorldManager* WorldManager;

    FVoxel* VoxelData;

private:
    static FastNoiseLite Noise;
    FVector GetWorldPositionFromVoxelIndex(int X, int Y, int Z) const;
    FIntVector GetVoxelIndexFromWorldPosition(const FVector& WorldPos) const;
    FIntVector GetWorldVoxelCoordinates(int LocalX, int LocalY, int LocalZ) const;
};