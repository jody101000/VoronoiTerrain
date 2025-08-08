// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelWorld.h"
#include "VoxelWorldManager.h"

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
            VoxelWorldManager->GenerateSolidCube(WorldPosition, CubeSizeX, CubeSizeY, CubeSizeZ);
            // VoxelWorldManager->GenerateSolidSphere(WorldPosition - FVector(0, 0, 1000), CubeSizeX, CubeSizeY, CubeSizeZ);

        }
    }
}

void AVoxelWorld::SculptAtPosition(const FVector& WorldPosition, float BrushStrength)
{
    if (VoxelWorldManager)
    {
        VoxelWorldManager->SculptAtPosition(WorldPosition, BrushStrength, CurrentMaterialId);
    }
}