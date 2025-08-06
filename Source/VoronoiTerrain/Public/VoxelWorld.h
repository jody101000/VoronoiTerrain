// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorld.generated.h"

class UVoxelWorldManager;

UCLASS()
class VORONOITERRAIN_API AVoxelWorld : public AActor
{
    GENERATED_BODY()

public:
    AVoxelWorld();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
    UMaterialInstance* ChunkMaterial;

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

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
    UVoxelWorldManager* VoxelWorldManager;

    UFUNCTION(BlueprintCallable)
    void SculptAtPosition(const FVector& WorldPosition, float StrengthMultiplier);
};