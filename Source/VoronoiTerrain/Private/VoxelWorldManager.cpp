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
    SculptBrush->Strength = BrushStrength; // Negative = add material
}

void UVoxelWorldManager::SculptAtPosition(const FVector& WorldPosition)
{
    if (!SculptBrush)
        return;

    // Update brush properties
    SculptBrush->Location = WorldPosition;
    SculptBrush->Strength = BrushStrength;

    if (USphereShape* SphereShape = Cast<USphereShape>(SculptBrush->Shape))
    {
        SphereShape->Radius = BrushRadius;
    }

    // Get all chunks that might be affected by this brush
    TArray<FIntVector> AffectedChunks = GetAffectedChunkCoordinates(WorldPosition, BrushRadius);

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
    return FIntVector(
        FMath::FloorToInt(WorldPos.X / ChunkWorldSize),
        FMath::FloorToInt(WorldPos.Y / ChunkWorldSize),
        FMath::FloorToInt(WorldPos.Z / ChunkWorldSize)
    );
}

UDynamicVoxelChunk* UVoxelWorldManager::GetOrCreateChunk(const FIntVector& ChunkCoords)
{
    if (UDynamicVoxelChunk** ExistingChunk = ActiveChunks.Find(ChunkCoords))
    {
        return *ExistingChunk;
    }

    // Create new chunk
    UDynamicVoxelChunk* NewChunk = NewObject<UDynamicVoxelChunk>(GetOwner());
    NewChunk->AttachToComponent(GetOwner()->GetRootComponent(),
        FAttachmentTransformRules::KeepWorldTransform);
    NewChunk->Material = ChunkMaterial;
    NewChunk->Initialize(ChunkCoords, VoxelSize);
    NewChunk->RegisterComponent();

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