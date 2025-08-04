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
    float VoxelSize = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrushRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrushStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInstance* ChunkMaterial;

    UFUNCTION(BlueprintCallable)
    void SculptAtPosition(const FVector& WorldPosition);

    UFUNCTION(BlueprintCallable)
    void ClearAllChunks();

private:
    UPROPERTY()
    TMap<FIntVector, UDynamicVoxelChunk*> ActiveChunks;

    UPROPERTY()
    UVoxelBrush* SculptBrush;

    FIntVector GetChunkCoordinatesFromWorldPosition(const FVector& WorldPos) const;
    UDynamicVoxelChunk* GetOrCreateChunk(const FIntVector& ChunkCoords);
    TArray<FIntVector> GetAffectedChunkCoordinates(const FVector& WorldPos, float Radius) const;
};