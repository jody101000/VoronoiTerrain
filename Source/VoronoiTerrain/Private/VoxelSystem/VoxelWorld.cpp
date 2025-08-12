#include "VoxelSystem/VoxelWorld.h"
#include "VoxelSystem/VoxelBrush.h"
#include "VoxelSystem/VoxelShape.h"
#include "VoxelSystem/DynamicVoxelChunk.h"
#include "Player/ClayBuilder.h"
#include "Utils/FastNoiseLite.h"

FastNoiseLite AVoxelWorld::Noise = FastNoiseLite();

AVoxelWorld::AVoxelWorld()
{
    PrimaryActorTick.bCanEverTick = true;
    USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    SetRootComponent(DefaultSceneRoot);
}

void AVoxelWorld::BeginPlay()
{
    Super::BeginPlay();
    // Create sculpt brush
    SculptBrush = NewObject<UVoxelBrush>();
    UVoxelShape* VoxelShape = NewObject<UVoxelShape>();
    VoxelShape->Radius = BrushRadius;
    SculptBrush->Shape = VoxelShape;
    SculptBrush->Strength = 1;

    if (bGenerateInitialCube)
    {
        FVector WorldPosition = GetActorLocation() + CubePosition;
        GenerateSolidCube(WorldPosition, CubeSizeX, CubeSizeY, CubeSizeZ, CurrentMaterialId);
        // FVector WorldPosition2 = WorldPosition - FVector(0.0f, CubeSizeY * ChunkSize * 1.5, 0.0f);
        // GenerateSolidCube(WorldPosition2, CubeSizeX / 2, CubeSizeY / 2, CubeSizeZ / 2, 2);
        // FVector WorldPosition3 = WorldPosition + FVector(0.0f, CubeSizeY * ChunkSize, 0.0f);
        // GenerateSolidCube(WorldPosition3, CubeSizeX / 2, CubeSizeY / 2, CubeSizeZ / 2, 3);
    }
}

void AVoxelWorld::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDecayEnabled)
    {
        CurrentDecayPosition += DecaySpeed * DeltaTime;
        
        DecayUpdateTimer += DeltaTime;
        if (DecayUpdateTimer >= DecayUpdateInterval)
        {
            DecayUpdateTimer = 0.0f;
            ProcessDecay();
            // UE_LOG(LogTemp, Warning, TEXT("Process decay"));
            
            DecaySpeed = FMath::Min(DecaySpeed + 0.5, 200);
        }
    }
}


void AVoxelWorld::StartDecay(FVector StartPosition, FVector Direction)
{
    DecayStartPosition = StartPosition;
    DecayDirection = Direction.GetSafeNormal();
    CurrentDecayPosition = 0.0f;
    bDecayEnabled = true;
}

void AVoxelWorld::StopDecay() 
{
    bDecayEnabled = false;
    CurrentDecayPosition = 0.0f;
}

void AVoxelWorld::ProcessDecay()
{
    FVector CurrentScanlinePosition = DecayStartPosition + DecayDirection * CurrentDecayPosition;
    
    // Calculate affected chunk range
    float ChunkWorldSize = ChunkSize * VoxelSize;
    int32 CurrentChunkX = FMath::FloorToInt(CurrentScanlinePosition.X / ChunkWorldSize);
    
    // Get chunks near scanline
    TArray<UDynamicVoxelChunk*> ChunksToUpdate;
    
    for (auto& ChunkPair : ActiveChunks)
    {
        if (FMath::Abs(ChunkPair.Key.X - CurrentChunkX) > 1 || CurrentChunkX - ChunkPair.Key.X > 3)
            continue;
            
        UDynamicVoxelChunk* Chunk = ChunkPair.Value;
        if (ProcessChunkDecay(Chunk))
        {
            ChunksToUpdate.Add(Chunk);
        }
    }
    
    for (UDynamicVoxelChunk* Chunk : ChunksToUpdate)
    {
        Chunk->UpdateMesh();
    }
}

bool AVoxelWorld::ProcessChunkDecay(UDynamicVoxelChunk* Chunk)
{
    if (!Chunk || !Chunk->VoxelData) return false;

    bool bChunkModified = false;
    FVector CurrentPlanePosition = DecayStartPosition + DecayDirection * CurrentDecayPosition;
    
    // Calculate X range to process
    float ChunkMinX = Chunk->ChunkCoordinates.X * ChunkSize * VoxelSize;
    int32 MaxX = FMath::Min(
        FMath::CeilToInt((CurrentPlanePosition.X - ChunkMinX) / VoxelSize) + 1,
        ChunkSize
    );
    
    if (MaxX <= 0) return false; // Scanline hasn't reached this chunk yet
    
    // Only process voxels up to the scanline position
    for (int x = 0; x < MaxX; x++)
    {
        for (int y = 0; y < ChunkSize; y++)
        {
            for (int z = 0; z < ChunkSize; z++)
            {
                int Index = x + ChunkSize * (y + ChunkSize * z);
                float& Density = Chunk->VoxelData[Index].Density;
                
                // Skip already empty voxels
                if (Density > 0) continue;
                
                // Simple decay without world position calculation
                Density = FMath::Min(1.0f, Density + 1.0f);
                bChunkModified = true;
            }
        }
    }
    
    return bChunkModified;
}

int32 AVoxelWorld::SculptAtPosition(const FVector& WorldPosition, float BrushStrength)
{
    if (!SculptBrush)
    {
        return 0;
    }

    int32 TotalVoxelChanges = 0;

    // Update brush properties
    SculptBrush->Location = WorldPosition;
    SculptBrush->Strength = BrushStrength;
    SculptBrush->MaterialId = CurrentMaterialId;
    SculptBrush->VoxelSize = VoxelSize;

    float BrushRadiusSet = (BrushStrength == 1) ? BrushRadius : EraseBrushRadius;
    SculptBrush->Shape->Radius = BrushRadiusSet;

    // Sculpt in all affected chunks
    TArray<FIntVector> AffectedChunks = GetAffectedChunkCoordinates(WorldPosition, BrushRadiusSet);
    for (const FIntVector& ChunkCoords : AffectedChunks)
    {
        UDynamicVoxelChunk* Chunk = GetOrCreateChunk(ChunkCoords);
        if (Chunk)
        {
            TotalVoxelChanges += Chunk->Sculpt(SculptBrush);
        }
    }

    return TotalVoxelChanges;
}

UDynamicVoxelChunk* AVoxelWorld::GetOrCreateChunk(const FIntVector& ChunkCoords)
{
    if (UDynamicVoxelChunk** ExistingChunk = ActiveChunks.Find(ChunkCoords))
    {
        return *ExistingChunk;
    }

    // Create new chunk
    UDynamicVoxelChunk* NewChunk = NewObject<UDynamicVoxelChunk>(this);
    NewChunk->Material = ChunkMaterial;
    NewChunk->VoxelWorld = this;

    NewChunk->RegisterComponent();
    NewChunk->AttachToComponent(GetRootComponent(),
        FAttachmentTransformRules::KeepWorldTransform);
    NewChunk->Initialize(ChunkCoords, ChunkSize, VoxelSize);

    ActiveChunks.Add(ChunkCoords, NewChunk);

    return NewChunk;
}

TArray<FIntVector> AVoxelWorld::GetAffectedChunkCoordinates(const FVector& WorldPos, float Radius) const
{
    TArray<FIntVector> AffectedChunks;

    float ChunkWorldSize = ChunkSize * VoxelSize;
    int32 ChunkRadius = FMath::CeilToInt(Radius / ChunkWorldSize);  // Number of chunks in a brush radius

    FIntVector CenterChunk = GetChunkCoordinatesFromWorldPosition(WorldPos);

    for (int32 x = -ChunkRadius; x <= ChunkRadius; x++)
    {
        for (int32 y = -ChunkRadius; y <= ChunkRadius; y++)
        {
            for (int32 z = -ChunkRadius; z <= ChunkRadius; z++)
            {
                AffectedChunks.Add(CenterChunk + FIntVector(x, y, z));
            }
        }
    }

    return AffectedChunks;
}

void AVoxelWorld::GenerateSolidCube(const FVector& Position, int32 SizeX, int32 SizeY, int32 SizeZ, int32 MaterialId)
{
    Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    Noise.SetFrequency(0.03f);
    Noise.SetSeed(1000);

    TSet<FIntVector> AffectedChunks;

    FIntVector StartVoxel = FIntVector(
        FMath::FloorToInt(Position.X / VoxelSize),
        FMath::FloorToInt(Position.Y / VoxelSize),
        FMath::FloorToInt(Position.Z / VoxelSize)
    );

    // Generate the cube
    for (int32 x = 0; x < SizeX; x++)
    {
        for (int32 y = 0; y < SizeY; y++)
        {
            float NoiseValue = Noise.GetNoise((float)(StartVoxel.X + x), (float)(StartVoxel.Y + y));
            NoiseValue = (NoiseValue + 1.0f) * 0.5f;
            float HeightOffset = NoiseValue * 5.0f;
            float SurfaceHeight = (SizeZ - 1) + HeightOffset;
            int32 MaxZ = FMath::CeilToInt(SurfaceHeight + 2.0f); // Padding for smooth transition
            
            for (int32 z = 0; z <= MaxZ; z++)
            {
                FIntVector WorldVoxelCoords = StartVoxel + FIntVector(x, y, z);
                FIntVector ChunkCoords = WorldVoxelCoordsToChunkCoords(WorldVoxelCoords);
                FIntVector LocalCoords = WorldVoxelCoordsToLocalCoords(WorldVoxelCoords);

                UDynamicVoxelChunk* Chunk = GetOrCreateChunk(ChunkCoords);
                if (Chunk && Chunk->VoxelData)
                {
                    int32 Index = LocalCoords.X + ChunkSize * (LocalCoords.Y + ChunkSize * LocalCoords.Z);

                    float DistanceFromSurface = z - SurfaceHeight;
                    float Density = DistanceFromSurface;

                    Chunk->VoxelData[Index].Density = Density;
                    Chunk->VoxelData[Index].MaterialId = MaterialId;
                    AffectedChunks.Add(ChunkCoords);
                }
            }
        }
    }

    // Update meshes
    for (const FIntVector& ChunkCoords : AffectedChunks)
    {
        if (UDynamicVoxelChunk* Chunk = GetOrCreateChunk(ChunkCoords))
        {
            Chunk->UpdateMesh();
        }
    }
}

void AVoxelWorld::GenerateSolidSphere(const FVector& Position, int32 SizeX, int32 SizeY, int32 SizeZ)
{
    TSet<FIntVector> AffectedChunks;

    float Radius = static_cast<float>(SizeX);
    FVector CenterVoxel = FVector(
        Position.X / VoxelSize,
        Position.Y / VoxelSize,
        Position.Z / VoxelSize
    );

    int32 VoxelRadius = FMath::CeilToInt(Radius);

    // Generate the sphere
    for (int32 x = -VoxelRadius; x <= VoxelRadius; x++)
    {
        for (int32 y = -VoxelRadius; y <= VoxelRadius; y++)
        {
            float NoiseValue = Noise.GetNoise((float)(CenterVoxel.X + x), (float)(CenterVoxel.Y + y));
            NoiseValue = (NoiseValue + 1.0f) * 0.5f;
            float HeightOffset = NoiseValue * 5.0f;
            float SurfaceHeight = (SizeZ - 1) + HeightOffset;
            int32 MaxZ = FMath::CeilToInt(SurfaceHeight + 2.0f);

            for (int32 z = -VoxelRadius; z <= MaxZ; z++)
            {
                FVector VoxelPos = FVector(x, y, z);
                if (VoxelPos.Size() <= Radius)
                {
                    FIntVector WorldVoxelCoords = FIntVector(
                        FMath::FloorToInt(CenterVoxel.X) + x,
                        FMath::FloorToInt(CenterVoxel.Y) + y,
                        FMath::FloorToInt(CenterVoxel.Z) + z
                    );

                    FIntVector ChunkCoords = WorldVoxelCoordsToChunkCoords(WorldVoxelCoords);
                    FIntVector LocalCoords = WorldVoxelCoordsToLocalCoords(WorldVoxelCoords);

                    UDynamicVoxelChunk* Chunk = GetOrCreateChunk(ChunkCoords);
                    if (Chunk && Chunk->VoxelData)
                    {
                        int32 Index = LocalCoords.X + ChunkSize * (LocalCoords.Y + ChunkSize * LocalCoords.Z);
                        float DistanceFromSurface = FVector::Distance(VoxelPos, Position) - Radius;

                        Chunk->VoxelData[Index].Density = -DistanceFromSurface;
                        Chunk->VoxelData[Index].MaterialId = CurrentMaterialId;
                        AffectedChunks.Add(ChunkCoords);
                    }
                }
            }
        }
    }

    // Update meshes
    for (const FIntVector& ChunkCoords : AffectedChunks)
    {
        if (UDynamicVoxelChunk* Chunk = GetOrCreateChunk(ChunkCoords))
        {
            Chunk->UpdateMesh();
        }
    }
}

int32 AVoxelWorld::GetTextureIdAtWorldPosition(const FVector& WorldPosition) const
{
    FIntVector WorldVoxelCoords = FIntVector(
        FMath::FloorToInt(WorldPosition.X / VoxelSize),
        FMath::FloorToInt(WorldPosition.Y / VoxelSize),
        FMath::FloorToInt(WorldPosition.Z / VoxelSize)
    );

    FVoxel VoxelData = GetVoxelAtWorldCoordinates(WorldVoxelCoords);

    return VoxelData.MaterialId;
}

FIntVector AVoxelWorld::GetChunkCoordinatesFromWorldPosition(const FVector& WorldPos) const
{
    float ChunkWorldSize = ChunkSize * VoxelSize;
    return FIntVector(
        FMath::FloorToInt(WorldPos.X / ChunkWorldSize),
        FMath::FloorToInt(WorldPos.Y / ChunkWorldSize),
        FMath::FloorToInt(WorldPos.Z / ChunkWorldSize)
    );
}

FVoxel AVoxelWorld::GetVoxelAtWorldCoordinates(const FIntVector& WorldVoxelCoords) const
{
    FIntVector ChunkCoords = WorldVoxelCoordsToChunkCoords(WorldVoxelCoords);
    FIntVector LocalCoords = WorldVoxelCoordsToLocalCoords(WorldVoxelCoords);

    // Check if the chunk exists
    if (UDynamicVoxelChunk* const* ChunkPtr = ActiveChunks.Find(ChunkCoords))
    {
        UDynamicVoxelChunk* Chunk = *ChunkPtr;
        if (Chunk && Chunk->VoxelData)
        {
            int Index = LocalCoords.X + ChunkSize * (LocalCoords.Y + ChunkSize * LocalCoords.Z);
            return Chunk->VoxelData[Index];
        }
    }

    // Return empty voxel
    return FVoxel(1.0f, 0, 0);
}

FIntVector AVoxelWorld::WorldVoxelCoordsToChunkCoords(const FIntVector& WorldVoxelCoords) const
{
    return FIntVector(
        WorldVoxelCoords.X >= 0 ? WorldVoxelCoords.X / ChunkSize : (WorldVoxelCoords.X - ChunkSize + 1) / ChunkSize,
        WorldVoxelCoords.Y >= 0 ? WorldVoxelCoords.Y / ChunkSize : (WorldVoxelCoords.Y - ChunkSize + 1) / ChunkSize,
        WorldVoxelCoords.Z >= 0 ? WorldVoxelCoords.Z / ChunkSize : (WorldVoxelCoords.Z - ChunkSize + 1) / ChunkSize
    );
}

FIntVector AVoxelWorld::WorldVoxelCoordsToLocalCoords(const FIntVector& WorldVoxelCoords) const
{
    FIntVector ChunkCoords = WorldVoxelCoordsToChunkCoords(WorldVoxelCoords);
    return WorldVoxelCoords - ChunkCoords * ChunkSize;
}