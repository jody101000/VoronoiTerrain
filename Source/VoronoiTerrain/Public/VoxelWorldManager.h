#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DynamicVoxelChunk.h"
#include "VoxelBrush.h"
#include "VoxelWorldManager.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VORONOITERRAIN_API UVoxelWorldManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVoxelWorldManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int ChunkSize = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VoxelSize = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrushRadius = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EraseBrushRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInstance* ChunkMaterial;

    UFUNCTION(BlueprintCallable)
    void SculptAtPosition(const FVector& WorldPosition, float BrushStrength, int32 MaterialId);

    UFUNCTION(BlueprintCallable)
    void ClearAllChunks();

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

    UPROPERTY()
    UVoxelBrush* EraseBrush;

    FIntVector GetChunkCoordinatesFromWorldPosition(const FVector& WorldPos) const;
    UDynamicVoxelChunk* GetOrCreateChunk(const FIntVector& ChunkCoords);
    TArray<FIntVector> GetAffectedChunkCoordinates(const FVector& WorldPos, float Radius) const;

    FIntVector WorldVoxelCoordsToChunkCoords(const FIntVector& WorldVoxelCoords) const;
    FIntVector WorldVoxelCoordsToLocalCoords(const FIntVector& WorldVoxelCoords) const;
};