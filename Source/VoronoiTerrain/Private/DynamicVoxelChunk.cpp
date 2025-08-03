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
    FMCMesh MeshData = MeshBuilder.Build(VoxelData, ChunkSize - 1);

    // Clear existing mesh
    FDynamicMesh3* Mesh = MeshComponent->GetMesh();
    Mesh->Clear();
    Mesh->EnableVertexNormals(FVector3f::ZeroVector);
    Mesh->EnableVertexColors(FVector4f::One());

    if (MeshData.Vertices.Num() == 0)
    {
        MeshComponent->NotifyMeshUpdated();
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

    UE_LOG(LogTemp, Warning, TEXT("Mesh Updated at (%.03f, %.03f, %.03f) with %d triangles"),
        FirstVertex.X, FirstVertex.Y, FirstVertex.Z, MeshData.Triangles.Num());

    MeshComponent->NotifyMeshUpdated();
    MeshComponent->UpdateCollision(false);

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
    return GetComponentLocation() + LocalPos;
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