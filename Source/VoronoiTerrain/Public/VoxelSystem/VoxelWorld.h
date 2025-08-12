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

    static FastNoiseLite Noise;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable)
    int32 SculptAtPosition(const FVector& WorldPosition, float BrushStrength);

    UFUNCTION(BlueprintCallable, Category = "Voxel Query")
    int32 GetTextureIdAtWorldPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Voxel Query")
    FVoxel GetVoxelAtWorldCoordinates(const FIntVector& WorldVoxelCoords) const;

    UFUNCTION(BlueprintCallable)
    void GenerateSolidCube(const FVector& Position, int32 SizeX, int32 SizeY, int32 SizeZ, int32 MaterialId);

    UFUNCTION(BlueprintCallable)
    void GenerateSolidSphere(const FVector& Position, int32 SizeX, int32 SizeY, int32 SizeZ);
    
    UFUNCTION(BlueprintCallable, Category = "Decay System")
    void StartDecay(FVector StartPosition, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "Decay System")
    void StopDecay();

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void SetCurrentMaterial(int32 MaterialId) { CurrentMaterialId = FMath::Clamp(MaterialId, 0, 3); }

    
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay System")
    bool bDecayEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay System")
    float DecaySpeed = 100.0f; // Units per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay System")
    FVector DecayDirection = FVector(1, 0, 0); // Scanline direction

    UPROPERTY(BlueprintReadOnly, Category = "Decay System")
    float CurrentDecayPosition = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay System")
    FVector DecayStartPosition;

private:    
    UDynamicVoxelChunk* GetOrCreateChunk(const FIntVector& ChunkCoords);
    TArray<FIntVector> GetAffectedChunkCoordinates(const FVector& WorldPos, float Radius) const;
    
    // --- Voxel, voxel chunk coordinate system helpers --- //
    FIntVector GetChunkCoordinatesFromWorldPosition(const FVector& WorldPos) const;
    FIntVector WorldVoxelCoordsToChunkCoords(const FIntVector& WorldVoxelCoords) const;
    FIntVector WorldVoxelCoordsToLocalCoords(const FIntVector& WorldVoxelCoords) const;

    void ProcessDecay();
    bool ProcessChunkDecay(UDynamicVoxelChunk* Chunk);

    UPROPERTY()
    TMap<FIntVector, UDynamicVoxelChunk*> ActiveChunks;

    UPROPERTY()
    UVoxelBrush* SculptBrush;
    
    float DecayUpdateInterval = 0.1f;
    float DecayUpdateTimer = 0.0f;
};