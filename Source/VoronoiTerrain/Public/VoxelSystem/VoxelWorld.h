#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "DynamicVoxelChunk.h"
#include "VoxelWorld.generated.h"

class UVoxelBrush;
class FastNoiseLite;

UCLASS()
class VORONOITERRAIN_API AVoxelWorld : public AActor
{
    GENERATED_BODY()

public:
    AVoxelWorld();
    virtual void BeginPlay() override;
    static FastNoiseLite Noise;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
    UMaterialInstance* ChunkMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
    int32 ChunkSize = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
    float VoxelSize = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Settings")
    float BrushRadius = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Settings")
    float EraseBrushRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initial Generation")
    bool bGenerateInitialCube = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initial Generation", meta = (ClampMin = "1"))
    int32 CubeSizeX = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initial Generation", meta = (ClampMin = "1"))
    int32 CubeSizeY = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initial Generation", meta = (ClampMin = "1"))
    int32 CubeSizeZ = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initial Generation")
    FVector CubePosition = FVector(0, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    int32 CurrentMaterialId = 0;

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void SetCurrentMaterial(int32 MaterialId) { CurrentMaterialId = FMath::Clamp(MaterialId, 0, 3); }

    UFUNCTION(BlueprintCallable)
    int32 SculptAtPosition(const FVector& WorldPosition, float BrushStrength);

    UFUNCTION(BlueprintCallable, Category = "Voxel Query")
    int32 GetTextureIdAtWorldPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable)
    FVoxel GetVoxelAtWorldCoordinates(const FIntVector& WorldVoxelCoords) const;

    UFUNCTION(BlueprintCallable)
    void GenerateSolidCube(const FVector& Position, int32 SizeX, int32 SizeY, int32 SizeZ);

    UFUNCTION(BlueprintCallable)
    void GenerateSolidSphere(const FVector& Position, int32 SizeX, int32 SizeY, int32 SizeZ);

private:
    UPROPERTY()
    TMap<FIntVector, UDynamicVoxelChunk*> ActiveChunks;

    UPROPERTY()
    UVoxelBrush* SculptBrush;

    FIntVector GetChunkCoordinatesFromWorldPosition(const FVector& WorldPos) const;
    UDynamicVoxelChunk* GetOrCreateChunk(const FIntVector& ChunkCoords);
    TArray<FIntVector> GetAffectedChunkCoordinates(const FVector& WorldPos, float Radius) const;

    FIntVector WorldVoxelCoordsToChunkCoords(const FIntVector& WorldVoxelCoords) const;
    FIntVector WorldVoxelCoordsToLocalCoords(const FIntVector& WorldVoxelCoords) const;
};