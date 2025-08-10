#include "VoxelSystem/DynamicVoxelChunk.h"
#include "VoxelSystem/VoxelWorld.h"
#include "VoxelSystem/VoxelShape.h"
#include "MarchingCubes/MeshBuilder.h"
#include "GeometryScript/CollisionFunctions.h"
#include "../VoronoiTerrainCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

FastNoiseLite UDynamicVoxelChunk::Noise = FastNoiseLite();

UDynamicVoxelChunk::UDynamicVoxelChunk()
{
    PrimaryComponentTick.bCanEverTick = false;
    VoxelData = nullptr;
    VoxelWorld = nullptr;
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
    }
    
    // Setup collisions

    MeshComponent->SetComplexAsSimpleCollisionEnabled(true, true);
    MeshComponent->bUseAsyncCooking = true;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    MeshComponent->SetGenerateOverlapEvents(true);
    MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
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

void UDynamicVoxelChunk::Initialize(FIntVector InChunkCoordinates, float InChunkSize, float InVoxelSize)
{
    ChunkCoordinates = InChunkCoordinates;
    VoxelSize = InVoxelSize;
    ChunkSize = InChunkSize;
    // Allocate voxel data
    int TotalVoxels = ChunkSize * ChunkSize * ChunkSize;
    VoxelData = new FVoxel[TotalVoxels];

    // Initialize all voxels as empty (positive density means empty space)
    for (int i = 0; i < TotalVoxels; i++)
    {
        VoxelData[i] = FVoxel(1.0f, 0, 0); // Positive = empty, negative = solid
    }

    // Set world position
    FVector WorldPos = FVector(
        ChunkCoordinates.X * ChunkSize * VoxelSize,
        ChunkCoordinates.Y * ChunkSize * VoxelSize,
        ChunkCoordinates.Z * ChunkSize * VoxelSize
    );
    SetWorldLocation(WorldPos);

    ChunkWorldOrigin = GetComponentLocation();
    bHasSetOrigin = true;

    //UE_LOG(LogTemp, Warning, TEXT("VoxelChunk Initialize: Chunk (%d, %d, %d) initialized at world position (%.2f, %.2f, %.2f)"),
    //    ChunkCoordinates.X, ChunkCoordinates.Y, ChunkCoordinates.Z,
    //    WorldPos.X, WorldPos.Y, WorldPos.Z);

    //// Verify the actual component location after setting
    //FVector ActualLocation = GetComponentLocation();
    //UE_LOG(LogTemp, Warning, TEXT("VoxelChunk Initialize: Chunk actual component location: (%.2f, %.2f, %.2f)"),
    //    ActualLocation.X, ActualLocation.Y, ActualLocation.Z);
}

int32 UDynamicVoxelChunk::Sculpt(UVoxelBrush* VoxelBrush)
{
    if (!VoxelData || !VoxelBrush)
        return 0;

    FVector BrushPos = VoxelBrush->Location;
    float Radius = VoxelBrush->Shape->Radius;
    FIntVector MinVoxelIndex = GetVoxelIndexFromWorldPosition(BrushPos - FVector(Radius) * 1.5f);
    FIntVector MaxVoxelIndex = GetVoxelIndexFromWorldPosition(BrushPos + FVector(Radius) * 1.5f);

    MinVoxelIndex.X = FMath::Clamp(MinVoxelIndex.X, 0, ChunkSize);
    MinVoxelIndex.Y = FMath::Clamp(MinVoxelIndex.Y, 0, ChunkSize);
    MinVoxelIndex.Z = FMath::Clamp(MinVoxelIndex.Z, 0, ChunkSize);

    MaxVoxelIndex.X = FMath::Clamp(MaxVoxelIndex.X, 0, ChunkSize);
    MaxVoxelIndex.Y = FMath::Clamp(MaxVoxelIndex.Y, 0, ChunkSize);
    MaxVoxelIndex.Z = FMath::Clamp(MaxVoxelIndex.Z, 0, ChunkSize);

    bool bModified = false;
    int32 VoxelStateChanges = 0;


    for (int x = MinVoxelIndex.X; x < MaxVoxelIndex.X; x++)
    {
        for (int y = MinVoxelIndex.Y; y < MaxVoxelIndex.Y; y++)
        {
            for (int z = MinVoxelIndex.Z; z < MaxVoxelIndex.Z; z++)
            {
                int Index = x + ChunkSize * (y + ChunkSize * z);
                FVector VoxelWorldPos = GetWorldPositionFromVoxelIndex(x, y, z);

                float OldDensity = VoxelData[Index].Density;
                VoxelBrush->Sculpt(VoxelData[Index], VoxelWorldPos);
                float NewDendity = VoxelData[Index].Density;

                if ((OldDensity > 0.0f && NewDendity <= 0.0f) || (OldDensity <= 0.0f && NewDendity > 0.0f))
                {
                    VoxelStateChanges++;
                }
                if (FMath::Abs(NewDendity - OldDensity) > 0.001f)
                {
                    VoxelBrush->Paint(VoxelData[Index], VoxelWorldPos);
                    bModified = true;
                }
            }
        }
    }

    if (bModified)
    {
        UpdateMesh();
    }

    return VoxelStateChanges;
}

void UDynamicVoxelChunk::UpdateMesh()
{
    if (!VoxelData || !MeshComponent || !VoxelWorld)
        return;
    
    int PaddedSize = ChunkSize + 1;
    int TotalPaddedVoxels = PaddedSize * PaddedSize * PaddedSize;
    FVoxel* PaddedData = new FVoxel[TotalPaddedVoxels];

    // Fill the padded array
    for (int z = 0; z < PaddedSize; z++)
    {
        for (int y = 0; y < PaddedSize; y++)
        {
            for (int x = 0; x < PaddedSize; x++)
            {
                int PaddedIndex = x + PaddedSize * (y + PaddedSize * z);
                FIntVector WorldVoxelCoords = GetWorldVoxelCoordinates(x - 1, y - 1, z - 1);
                
                if (x >= 1 && x < ChunkSize + 1 && y >= 1 && y < ChunkSize + 1 && z >= 1 && z < ChunkSize + 1)
                {
                    // Use local chunk data
                    int LocalIndex = (x - 1) + ChunkSize * ((y - 1) + ChunkSize * (z - 1));
                    PaddedData[PaddedIndex] = VoxelData[LocalIndex];
                }
                else
                {
                    // Get data from neighboring chunks
                    PaddedData[PaddedIndex] = VoxelWorld->GetVoxelAtWorldCoordinates(WorldVoxelCoords);
                }
            }
        }
    }

    // Generate mesh with padded data
    FMCMeshBuilder MeshBuilder;
    FMCMesh MeshData = MeshBuilder.Build(PaddedData, ChunkSize, VoxelSize);
    delete[] PaddedData;

    // if (MeshData.Vertices.Num() == 0)
    // {
    //     MeshComponent->GetDynamicMesh()->EditMesh([&](FDynamicMesh3& Mesh)
    //     {
    //         Mesh.Clear();
    //         Mesh.EnableVertexNormals(FVector3f::ZeroVector);
    //         Mesh.EnableVertexColors(FVector4f::One());
    //     });
    //     MeshComponent->NotifyMeshModified();
    //     return;
    // }
    
    MeshComponent->GetDynamicMesh()->EditMesh([&](FDynamicMesh3& Mesh)
    {
        Mesh.Clear();
        Mesh.EnableVertexNormals(FVector3f());
        Mesh.EnableVertexColors(FVector4f());

        Mesh.EnableAttributes();
        Mesh.Attributes()->EnablePrimaryColors();
        const auto ColorOverlay = Mesh.Attributes()->PrimaryColors();

    
        // Add vertices, normals, colors
        TArray<int32> VertexIndices;
        for (int i = 0; i < MeshData.Vertices.Num(); i++)
        {
            int32 VertexId = Mesh.AppendVertex(MeshData.Vertices[i]);
            VertexIndices.Add(VertexId);
    
            Mesh.SetVertexNormal(VertexId, FVector3f(MeshData.Normals[i]));
            Mesh.SetVertexColor(VertexId, FVector4f(MeshData.Colors[i]));
            ColorOverlay->AppendElement(MeshData.Colors[i]);
        }
    
        // Add triangles
        for (int i = 0; i < MeshData.Triangles.Num(); i += 3)
        {
            int32 V0 = VertexIndices[MeshData.Triangles[i]];
            int32 V1 = VertexIndices[MeshData.Triangles[i + 1]];
            int32 V2 = VertexIndices[MeshData.Triangles[i + 2]];
            const int Id = Mesh.AppendTriangle(V0, V1, V2);
            ColorOverlay->SetTriangle(Id, UE::Geometry::FIndex3i(V0, V1, V2));
        }
    });
    
    MeshComponent->NotifyMeshModified();
    UGeometryScriptLibrary_CollisionFunctions::SetDynamicMeshCollisionFromMesh(MeshComponent->GetDynamicMesh(), MeshComponent, FGeometryScriptCollisionFromMeshOptions());
    MeshComponent->UpdateCollision(false);
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
    FVector LocalPos = FVector(X + 0.5f, Y + 0.5f, Z + 0.5f) * VoxelSize; // center
    return (bHasSetOrigin ? ChunkWorldOrigin : GetComponentLocation()) + LocalPos;
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

FIntVector UDynamicVoxelChunk::GetWorldVoxelCoordinates(int LocalX, int LocalY, int LocalZ) const
{
    return FIntVector(
        ChunkCoordinates.X * ChunkSize + LocalX,
        ChunkCoordinates.Y * ChunkSize + LocalY,
        ChunkCoordinates.Z * ChunkSize + LocalZ
    );
}

float UDynamicVoxelChunk::GetTextureIdAtLocalPosition(const FVector& LocalPosition) const
{
    if (!VoxelData)
        return 0.0f;

    // Convert local position to voxel indices
    FIntVector VoxelIndex = FIntVector(
        FMath::FloorToInt(LocalPosition.X / VoxelSize),
        FMath::FloorToInt(LocalPosition.Y / VoxelSize),
        FMath::FloorToInt(LocalPosition.Z / VoxelSize)
    );

    // Check bounds
    if (VoxelIndex.X < 0 || VoxelIndex.X >= ChunkSize ||
        VoxelIndex.Y < 0 || VoxelIndex.Y >= ChunkSize ||
        VoxelIndex.Z < 0 || VoxelIndex.Z >= ChunkSize)
    {
        return 0.0f; // Out of bounds
    }

    // Calculate array index
    int32 Index = VoxelIndex.X + ChunkSize * (VoxelIndex.Y + ChunkSize * VoxelIndex.Z);

    return VoxelData[Index].MaterialId;
}

float UDynamicVoxelChunk::GetTextureIdAtWorldPosition(const FVector& WorldPosition) const
{
    // Convert world position to local position
    FVector LocalPosition = WorldPosition - GetComponentLocation();
    return GetTextureIdAtLocalPosition(LocalPosition);
}