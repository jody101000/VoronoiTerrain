// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelSystem/VoxelWorld.h"
#include "VoxelSystem/VoxelWorldManager.h"

AVoxelWorld::AVoxelWorld()
{
    USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    SetRootComponent(DefaultSceneRoot);

    VoxelWorldManager = CreateDefaultSubobject<UVoxelWorldManager>(TEXT("VoxelWorldManager"));
}

void AVoxelWorld::BeginPlay()
{
    Super::BeginPlay();

    if (VoxelWorldManager)
    {
        VoxelWorldManager->ChunkMaterial = ChunkMaterial;
        VoxelWorldManager->BrushRadius = BrushRadius;
        VoxelWorldManager->EraseBrushRadius = EraseBrushRadius;

        if (bGenerateInitialCube)
        {
            FVector WorldPosition = GetActorLocation() + CubePosition; // CubePosition becomes offset
            VoxelWorldManager->GenerateSolidCube(WorldPosition, CubeSizeX, CubeSizeY, CubeSizeZ, CurrentMaterialId);
            // VoxelWorldManager->GenerateSolidSphere(WorldPosition - FVector(0, 0, 1000), CubeSizeX, CubeSizeY, CubeSizeZ);

        }
    }
}

int32 AVoxelWorld::SculptAtPosition(const FVector& WorldPosition, float BrushStrength)
{
    if (VoxelWorldManager)
    {
        return VoxelWorldManager->SculptAtPosition(WorldPosition, BrushStrength, CurrentMaterialId);
    }
    return 0;
}

int32 AVoxelWorld::GetTextureIdAtWorldPosition(const FVector& WorldPosition) const
{
    if (VoxelWorldManager)
    {
        return VoxelWorldManager->GetTextureIdAtWorldPosition(WorldPosition);
    }
    return 0;
}