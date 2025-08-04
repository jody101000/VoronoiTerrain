#include "DynamicVoxelChunk.h"
#include "MarchingCubes/MeshBuilder.h"
#include "SphereShape.h"

UDynamicVoxelChunk::UDynamicVoxelChunk()
{
    PrimaryComponentTick.bCanEverTick = false;
    VoxelData = nullptr;
    bNeedsUpdate = false;
}

void UDynamicVoxelChunk::BeginPlay()
{
    Super::BeginPlay();

    if (!MeshComponent)
    {
        MeshComponent = NewObject<UDynamicMeshComponent>(GetOwner());
        MeshComponent->RegisterComponent();
        MeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);

        if (Material)
        {
            MeshComponent->SetMaterial(0, Material);
        }

        MeshComponent->SetComplexAsSimpleCollisionEnabled(true, true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECC_WorldStatic); // Changed to WorldStatic for better ray tracing
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
        MeshComponent->SetGenerateOverlapEvents(false);
        MeshComponent->bUseAsyncCooking = false; // Disable async cooking for immediate collision updates
        MeshComponent->SetNotifyRigidBodyCollision(true);
    }
}

void UDynamicVoxelChunk::BeginDestroy()
{
    if (VoxelData)
    {
        delete[] VoxelData;
        VoxelData = nullptr;
    }
    Super::BeginDestroy();
}

void UDynamicVoxelChunk::Initialize(FIntVector InChunkCoordinates, float InVoxelSize)
{
    ChunkCoordinates = InChunkCoordinates;
    VoxelSize = InVoxelSize;

    // Allocate voxel data
    int TotalVoxels = ChunkSize * ChunkSize * ChunkSize;
    VoxelData = new FVoxel[TotalVoxels];

    // Initialize all voxels as empty (positive density means empty space)
    for (int i = 0; i < TotalVoxels; i++)
    {
        VoxelData[i] = FVoxel(1.0f, 0); // Positive = empty, negative = solid
    }

    // Set world position
    FVector WorldPos = FVector(
        ChunkCoordinates.X * ChunkSize * VoxelSize,
        ChunkCoordinates.Y * ChunkSize * VoxelSize,
        ChunkCoordinates.Z * ChunkSize * VoxelSize
    );
    SetWorldLocation(WorldPos);

    //UE_LOG(LogTemp, Warning, TEXT("VoxelChunk Initialize: Chunk (%d, %d, %d) initialized at world position (%.2f, %.2f, %.2f)"),
    //    ChunkCoordinates.X, ChunkCoordinates.Y, ChunkCoordinates.Z,
    //    WorldPos.X, WorldPos.Y, WorldPos.Z);

    //// Verify the actual component location after setting
    //FVector ActualLocation = GetComponentLocation();
    //UE_LOG(LogTemp, Warning, TEXT("VoxelChunk Initialize: Chunk actual component location: (%.2f, %.2f, %.2f)"),
    //    ActualLocation.X, ActualLocation.Y, ActualLocation.Z);
}

void UDynamicVoxelChunk::Sculpt(UVoxelBrush* VoxelBrush)
{
    if (!VoxelData || !VoxelBrush)
        return;

    bool bModified = false;

    for (int x = 0; x < ChunkSize; x++)
    {
        for (int y = 0; y < ChunkSize; y++)
        {
            for (int z = 0; z < ChunkSize; z++)
            {
                int Index = x + ChunkSize * (y + ChunkSize * z);
                FVector VoxelWorldPos = GetWorldPositionFromVoxelIndex(x, y, z);

                float OldDensity = VoxelData[Index].Density;
                VoxelBrush->Sculpt(VoxelData[Index], VoxelWorldPos);
                if (FMath::Abs(VoxelData[Index].Density - OldDensity) > 0.001f)
                {
                    bModified = true;
                }
            }
        }
    }

    if (bModified)
    {
        bNeedsUpdate = true;
        UpdateMesh();
    }
}

void UDynamicVoxelChunk::UpdateMesh()
{
    if (!bNeedsUpdate || !VoxelData || !MeshComponent)
        return;

    // Use marching cubes to generate mesh
    FMCMeshBuilder MeshBuilder;
    FMCMesh MeshData = MeshBuilder.Build(VoxelData, ChunkSize - 1, VoxelSize);

    // Clear existing mesh
    FDynamicMesh3* Mesh = MeshComponent->GetMesh();
    Mesh->Clear();
    Mesh->EnableVertexNormals(FVector3f::ZeroVector);
    Mesh->EnableVertexColors(FVector4f::One());

    if (MeshData.Vertices.Num() == 0)
    {
        MeshComponent->NotifyMeshUpdated();
        MeshComponent->UpdateCollision(true);
        return;
    }

    // Add vertices
    TArray<int32> VertexIndices;
    for (int i = 0; i < MeshData.Vertices.Num(); i++)
    {
        int32 VertexId = Mesh->AppendVertex(MeshData.Vertices[i]);
        VertexIndices.Add(VertexId);

        Mesh->SetVertexNormal(VertexId, FVector3f(MeshData.Normals[i]));
        Mesh->SetVertexColor(VertexId, FVector4f(MeshData.Colors[i]));
    }

    // Add triangles
    for (int i = 0; i < MeshData.Triangles.Num(); i += 3)
    {
        int32 V0 = VertexIndices[MeshData.Triangles[i]];
        int32 V1 = VertexIndices[MeshData.Triangles[i + 1]];
        int32 V2 = VertexIndices[MeshData.Triangles[i + 2]];
        Mesh->AppendTriangle(V0, V1, V2);
    }

    FVector FirstVertex = MeshData.Vertices[0];

    MeshComponent->NotifyMeshUpdated();
    MeshComponent->UpdateCollision(true);

    //FVector ChunkWorldPos = GetComponentLocation();
    //UE_LOG(LogTemp, Warning, TEXT("VoxelChunk UpdateMesh: Chunk (%d, %d, %d) at world pos (%.2f, %.2f, %.2f) updated mesh with %d triangles, %d vertices"),
    //    ChunkCoordinates.X, ChunkCoordinates.Y, ChunkCoordinates.Z,
    //    ChunkWorldPos.X, ChunkWorldPos.Y, ChunkWorldPos.Z,
    //    MeshData.Triangles.Num() / 3, MeshData.Vertices.Num());

    bNeedsUpdate = false;
}

bool UDynamicVoxelChunk::IsEmpty() const
{
    if (!VoxelData)
        return true;

    int TotalVoxels = ChunkSize * ChunkSize * ChunkSize;
    for (int i = 0; i < TotalVoxels; i++)
    {
        if (VoxelData[i].Density < 0.0f) // Negative density = solid
        {
            return false;
        }
    }
    return true;
}

FVector UDynamicVoxelChunk::GetWorldPositionFromVoxelIndex(int X, int Y, int Z) const
{
    FVector LocalPos = FVector(X, Y, Z) * VoxelSize;
    //return GetComponentLocation() + LocalPos;
    FVector WorldPos = GetComponentLocation() + LocalPos;

    // Debug for first few voxels only to avoid spam
    if (X < 2 && Y < 2 && Z < 2)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("VoxelChunk: Voxel (%d,%d,%d) -> Local (%.2f,%.2f,%.2f) -> World (%.2f,%.2f,%.2f)"),
            X, Y, Z, LocalPos.X, LocalPos.Y, LocalPos.Z, WorldPos.X, WorldPos.Y, WorldPos.Z);
    }

    return WorldPos;
}

FIntVector UDynamicVoxelChunk::GetVoxelIndexFromWorldPosition(const FVector& WorldPos) const
{
    FVector LocalPos = WorldPos - GetComponentLocation();
    return FIntVector(
        FMath::FloorToInt(LocalPos.X / VoxelSize),
        FMath::FloorToInt(LocalPos.Y / VoxelSize),
        FMath::FloorToInt(LocalPos.Z / VoxelSize)
    );
}