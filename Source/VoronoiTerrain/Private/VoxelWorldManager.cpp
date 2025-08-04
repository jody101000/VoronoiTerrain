#include "VoxelWorldManager.h"
#include "SphereShape.h"

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
    SphereShape->Radius = BrushRadius;
    SculptBrush->Shape = SphereShape;
    SculptBrush->Strength = BrushStrength;
}

void UVoxelWorldManager::SculptAtPosition(const FVector& WorldPosition)
{
    if (!SculptBrush)
        return;

    //UE_LOG(LogTemp, Warning, TEXT("VoxelWorldManager: Sculpting at World Position: (%.2f, %.2f, %.2f)"),
    //    WorldPosition.X, WorldPosition.Y, WorldPosition.Z);

    // Update brush properties
    SculptBrush->Location = WorldPosition;
    SculptBrush->Strength = BrushStrength;

    if (USphereShape* SphereShape = Cast<USphereShape>(SculptBrush->Shape))
    {
        SphereShape->Radius = BrushRadius;
    }

    // Debug: Log chunk coordinates
    FIntVector CenterChunk = GetChunkCoordinatesFromWorldPosition(WorldPosition);
    //UE_LOG(LogTemp, Warning, TEXT("VoxelWorldManager: Center Chunk Coordinates: (%d, %d, %d)"),
    //    CenterChunk.X, CenterChunk.Y, CenterChunk.Z);

    // Get all chunks that might be affected by this brush
    TArray<FIntVector> AffectedChunks = GetAffectedChunkCoordinates(WorldPosition, BrushRadius);
    
    //UE_LOG(LogTemp, Warning, TEXT("VoxelWorldManager: Affecting %d chunks"), AffectedChunks.Num());
    
    // Sculpt in all affected chunks
    for (const FIntVector& ChunkCoords : AffectedChunks)
    {
        UDynamicVoxelChunk* Chunk = GetOrCreateChunk(ChunkCoords);
        if (Chunk)
        {
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
    //return FIntVector(
    //    FMath::FloorToInt(WorldPos.X / ChunkWorldSize),
    //    FMath::FloorToInt(WorldPos.Y / ChunkWorldSize),
    //    FMath::FloorToInt(WorldPos.Z / ChunkWorldSize)
    //);
    FIntVector ChunkCoords = FIntVector(
        FMath::FloorToInt(WorldPos.X / ChunkWorldSize),
        FMath::FloorToInt(WorldPos.Y / ChunkWorldSize),
        FMath::FloorToInt(WorldPos.Z / ChunkWorldSize)
    );
    //return FIntVector(
    //    WorldPos.X >= 0 ? FMath::FloorToInt(WorldPos.X / ChunkWorldSize) : FMath::FloorToInt((WorldPos.X - ChunkWorldSize + 1) / ChunkWorldSize),
    //    WorldPos.Y >= 0 ? FMath::FloorToInt(WorldPos.Y / ChunkWorldSize) : FMath::FloorToInt((WorldPos.Y - ChunkWorldSize + 1) / ChunkWorldSize),
    //    WorldPos.Z >= 0 ? FMath::FloorToInt(WorldPos.Z / ChunkWorldSize) : FMath::FloorToInt((WorldPos.Z - ChunkWorldSize + 1) / ChunkWorldSize)
    //);
    //UE_LOG(LogTemp, Warning, TEXT("VoxelWorldManager: AffectedChunks World Pos: (%.2f, %.2f, %.2f) -> Chunk Size: %.2f -> Chunk Coords: (%d, %d, %d)"),
    //    WorldPos.X, WorldPos.Y, WorldPos.Z, ChunkWorldSize, ChunkCoords.X, ChunkCoords.Y, ChunkCoords.Z);

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

    NewChunk->RegisterComponent();

    NewChunk->Initialize(ChunkCoords, VoxelSize);

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