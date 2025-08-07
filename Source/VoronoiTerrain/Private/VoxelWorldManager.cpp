#include "VoxelWorldManager.h"
#include "SphereShape.h"
#include "CylinderShape.h"

UVoxelWorldManager::UVoxelWorldManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UVoxelWorldManager::BeginPlay()
{
    Super::BeginPlay();

    // Create sculpt brush
    SculptBrush = NewObject<UVoxelBrush>();
    USphereShape* SphereShape = NewObject<USphereShape>();
    //UCylinderShape* SphereShape = NewObject<USphereShape>();
    SphereShape->Radius = BrushRadius;
    SculptBrush->Shape = SphereShape;
    SculptBrush->Strength = 1;
}

void UVoxelWorldManager::SculptAtPosition(const FVector& WorldPosition, float BrushStrength, EVoxelPhysicsType PhysicsType)
{
    if (!SculptBrush)
        return;

    // UE_LOG(LogTemp, Warning, TEXT("VoxelWorldManager: Sculpting at World Position: (%.2f, %.2f, %.2f)"),
    //     WorldPosition.X, WorldPosition.Y, WorldPosition.Z);

    // Update brush properties
    SculptBrush->Location = WorldPosition;
    SculptBrush->Strength = BrushStrength;

    float BrushRadiusSet = (BrushStrength == 1) ? BrushRadius : EraseBrushRadius;

    if (USphereShape* SphereShape = Cast<USphereShape>(SculptBrush->Shape))
    {
        SphereShape->Radius = BrushRadiusSet;
    }

    // // Debug: Log chunk coordinates
    // FIntVector CenterChunk = GetChunkCoordinatesFromWorldPosition(WorldPosition);
    // UE_LOG(LogTemp, Warning, TEXT("VoxelWorldManager: Center Chunk Coordinates: (%d, %d, %d)"),
    //     CenterChunk.X, CenterChunk.Y, CenterChunk.Z);

    // Get all chunks that might be affected by this brush
    TArray<FIntVector> AffectedChunks = GetAffectedChunkCoordinates(WorldPosition, BrushRadiusSet);
    
    // UE_LOG(LogTemp, Warning, TEXT("VoxelWorldManager: Affecting %d chunks"), AffectedChunks.Num());
    
    // Sculpt in all affected chunks
    for (const FIntVector& ChunkCoords : AffectedChunks)
    {
        UDynamicVoxelChunk* Chunk = GetOrCreateChunk(ChunkCoords);
        if (Chunk)
        {
            Chunk->SetPhysicsType(PhysicsType);
            Chunk->Sculpt(SculptBrush);
        }
    }
}

void UVoxelWorldManager::ClearAllChunks()
{
    for (auto& ChunkPair : ActiveChunks)
    {
        if (ChunkPair.Value)
        {
            ChunkPair.Value->DestroyComponent();
        }
    }
    ActiveChunks.Empty();
}

FIntVector UVoxelWorldManager::GetChunkCoordinatesFromWorldPosition(const FVector& WorldPos) const
{
    float ChunkWorldSize = ChunkSize * VoxelSize;
    FIntVector ChunkCoords = FIntVector(
        FMath::FloorToInt(WorldPos.X / ChunkWorldSize),
        FMath::FloorToInt(WorldPos.Y / ChunkWorldSize),
        FMath::FloorToInt(WorldPos.Z / ChunkWorldSize)
    );

    // UE_LOG(LogTemp, Warning, TEXT("VoxelWorldManager: AffectedChunks World Pos: (%.2f, %.2f, %.2f) -> Chunk Size: %.2f -> Chunk Coords: (%d, %d, %d)"),
    //     WorldPos.X, WorldPos.Y, WorldPos.Z, ChunkWorldSize, ChunkCoords.X, ChunkCoords.Y, ChunkCoords.Z);

    return ChunkCoords;
}

UDynamicVoxelChunk* UVoxelWorldManager::GetOrCreateChunk(const FIntVector& ChunkCoords)
{
    if (UDynamicVoxelChunk** ExistingChunk = ActiveChunks.Find(ChunkCoords))
    {
        return *ExistingChunk;
    }

    // Create new chunk
    UDynamicVoxelChunk* NewChunk = NewObject<UDynamicVoxelChunk>(GetOwner());
    NewChunk->Material = ChunkMaterial;
    NewChunk->WorldManager = this;

    NewChunk->RegisterComponent();
    NewChunk->AttachToComponent(GetOwner()->GetRootComponent(),
        FAttachmentTransformRules::KeepWorldTransform);
    NewChunk->Initialize(ChunkCoords, VoxelSize);

    // ECollisionEnabled::Type collision = NewChunk->GetCollisionEnabled();
    // UE_LOG(LogTemp, Warning, TEXT("VoxelWorldManager: Chunk created with collision type %s"), *UEnum::GetValueAsString(collision));

    ActiveChunks.Add(ChunkCoords, NewChunk);
    return NewChunk;
}

TArray<FIntVector> UVoxelWorldManager::GetAffectedChunkCoordinates(const FVector& WorldPos, float Radius) const
{
    TArray<FIntVector> AffectedChunks;

    float ChunkWorldSize = ChunkSize * VoxelSize;
    int32 ChunkRadius = FMath::CeilToInt(Radius / ChunkWorldSize);

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

FVoxel UVoxelWorldManager::GetVoxelAtWorldCoordinates(const FIntVector& WorldVoxelCoords) const
{
    FIntVector ChunkCoords = WorldVoxelCoordsToChunkCoords(WorldVoxelCoords);
    FIntVector LocalCoords = WorldVoxelCoordsToLocalCoords(WorldVoxelCoords);

    // Check if the chunk exists
    if (UDynamicVoxelChunk* const* ChunkPtr = ActiveChunks.Find(ChunkCoords))
    {
        UDynamicVoxelChunk* Chunk = *ChunkPtr;
        if (Chunk && Chunk->VoxelData)
        {
            // Ensure local coordinates are within bounds
            if (LocalCoords.X >= 0 && LocalCoords.X < ChunkSize &&
                LocalCoords.Y >= 0 && LocalCoords.Y < ChunkSize &&
                LocalCoords.Z >= 0 && LocalCoords.Z < ChunkSize)
            {
                int Index = LocalCoords.X + ChunkSize * (LocalCoords.Y + ChunkSize * LocalCoords.Z);
                return Chunk->VoxelData[Index];
            }
        }
    }

    // Return empty voxel if chunk doesn't exist or coordinates are out of bounds
    return FVoxel(1.0f, 0); // Positive density = empty space
}

FIntVector UVoxelWorldManager::WorldVoxelCoordsToChunkCoords(const FIntVector& WorldVoxelCoords) const
{
    return FIntVector(
        WorldVoxelCoords.X >= 0 ? WorldVoxelCoords.X / ChunkSize : (WorldVoxelCoords.X - ChunkSize + 1) / ChunkSize,
        WorldVoxelCoords.Y >= 0 ? WorldVoxelCoords.Y / ChunkSize : (WorldVoxelCoords.Y - ChunkSize + 1) / ChunkSize,
        WorldVoxelCoords.Z >= 0 ? WorldVoxelCoords.Z / ChunkSize : (WorldVoxelCoords.Z - ChunkSize + 1) / ChunkSize
    );
}

FIntVector UVoxelWorldManager::WorldVoxelCoordsToLocalCoords(const FIntVector& WorldVoxelCoords) const
{
    FIntVector ChunkCoords = WorldVoxelCoordsToChunkCoords(WorldVoxelCoords);
    return FIntVector(
        WorldVoxelCoords.X - ChunkCoords.X * ChunkSize,
        WorldVoxelCoords.Y - ChunkCoords.Y * ChunkSize,
        WorldVoxelCoords.Z - ChunkCoords.Z * ChunkSize
    );
}

void UVoxelWorldManager::GenerateSolidCube(const FVector& Position, int32 SizeX, int32 SizeY, int32 SizeZ)
{
    TSet<FIntVector> AffectedChunks;

    // Convert world position to voxel coordinates
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
            for (int32 z = 0; z < SizeZ; z++)
            {
                FIntVector WorldVoxelCoords = StartVoxel + FIntVector(x, y, z);
                FIntVector ChunkCoords = WorldVoxelCoordsToChunkCoords(WorldVoxelCoords);
                FIntVector LocalCoords = WorldVoxelCoordsToLocalCoords(WorldVoxelCoords);

                // Get or create chunk
                UDynamicVoxelChunk* Chunk = GetOrCreateChunk(ChunkCoords);
                if (Chunk && Chunk->VoxelData)
                {
                    // Ensure coordinates are within chunk bounds
                    if (LocalCoords.X >= 0 && LocalCoords.X < ChunkSize &&
                        LocalCoords.Y >= 0 && LocalCoords.Y < ChunkSize &&
                        LocalCoords.Z >= 0 && LocalCoords.Z < ChunkSize)
                    {
                        int32 Index = LocalCoords.X + ChunkSize * (LocalCoords.Y + ChunkSize * LocalCoords.Z);
                        Chunk->VoxelData[Index].Density = -1.0f; // Solid voxel
                        AffectedChunks.Add(ChunkCoords);
                    }
                }
            }
        }
    }

    // Update meshes for all affected chunks
    for (const FIntVector& ChunkCoords : AffectedChunks)
    {
        if (UDynamicVoxelChunk* Chunk = GetOrCreateChunk(ChunkCoords))
        {
            Chunk->UpdateMesh();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Generated solid cube at (%.2f, %.2f, %.2f) with size (%d, %d, %d)"),
        Position.X, Position.Y, Position.Z, SizeX, SizeY, SizeZ);
}

void UVoxelWorldManager::GenerateSolidSphere(const FVector& Position, int32 SizeX, int32 SizeY, int32 SizeZ)
{
    TSet<FIntVector> AffectedChunks;
    
    float Radius = static_cast<float>(SizeX); // Use SizeX as radius
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
            for (int32 z = -VoxelRadius; z <= VoxelRadius; z++)
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
                        if (LocalCoords.X >= 0 && LocalCoords.X < ChunkSize &&
                            LocalCoords.Y >= 0 && LocalCoords.Y < ChunkSize &&
                            LocalCoords.Z >= 0 && LocalCoords.Z < ChunkSize)
                        {
                            int32 Index = LocalCoords.X + ChunkSize * (LocalCoords.Y + ChunkSize * LocalCoords.Z);
                            Chunk->VoxelData[Index].Density = -1.0f;
                            AffectedChunks.Add(ChunkCoords);
                        }
                    }
                }
            }
        }
    }

    // Update meshes for all affected chunks
    for (const FIntVector& ChunkCoords : AffectedChunks)
    {
        if (UDynamicVoxelChunk* Chunk = GetOrCreateChunk(ChunkCoords))
        {
            Chunk->UpdateMesh();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Generated solid sphere at (%.2f, %.2f, %.2f) with size (%d, %d, %d)"),
        Position.X, Position.Y, Position.Z, SizeX, SizeY, SizeZ);
}