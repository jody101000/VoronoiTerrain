// Fill out your copyright notice in the Description page of Project Settings.


#include "NaiveChunk.h"

#include "Voxel/Utils/FastNoiseLite.h"

void ANaiveChunk::Setup()
{
	// Initialize Blocks
	Blocks.SetNum(Size * Size * Size);
}

void ANaiveChunk::Generate2DHeightMap(const FVector Position)
{
	for (int x = 0; x < Size; x++)
	{
		for (int y = 0; y < Size; y++)
		{
			const float Xpos = x + Position.X;
			const float ypos = y + Position.Y;
			
			const int Height = FMath::Clamp(FMath::RoundToInt((Noise->GetNoise(Xpos, ypos) + 1) * Size / 2), 0, Size);

			for (int z = 0; z < Height; z++)
			{
				Blocks[GetBlockIndex(x,y,z)] = EVoxelTutorialBlock::Stone;
			}

			for (int z = Height; z < Size; z++)
			{
				Blocks[GetBlockIndex(x,y,z)] = EVoxelTutorialBlock::Air;
			}
			
		}
	}
}

void ANaiveChunk::Generate3DHeightMap(const FVector Position)
{
	for (int x = 0; x < Size; ++x)
	{
		for (int y = 0; y < Size; ++y)
		{
			for (int z = 0; z < Size; ++z)
			{
				const auto NoiseValue = Noise->GetNoise(x + Position.X, y + Position.Y, z + Position.Z);

				if (NoiseValue >= 0)
				{
					Blocks[GetBlockIndex(x, y, z)] = EVoxelTutorialBlock::Air;
				}
				else
				{
					Blocks[GetBlockIndex(x, y, z)] = EVoxelTutorialBlock::Stone;
				}
			}
		}
	}
}

void ANaiveChunk::GenerateMesh()
{
	for (int x = 0; x < Size; x++)
	{
		for (int y = 0; y < Size; y++)
		{
			for (int z = 0; z < Size; z++)
			{
				if (Blocks[GetBlockIndex(x,y,z)] != EVoxelTutorialBlock::Air)
				{
					const auto Position = FVector(x,y,z);
					
					for (auto Direction : {EVoxelTutorialDirection::Forward, EVoxelTutorialDirection::Right, EVoxelTutorialDirection::Back, EVoxelTutorialDirection::Left, EVoxelTutorialDirection::Up, EVoxelTutorialDirection::Down})
					{
						if (Check(GetPositionInDirection(Direction, Position)))
						{
							CreateFace(Direction, Position * 100);
						}
					}
				}
			}
		}
	}
}

bool ANaiveChunk::Check(const FVector Position) const
{
	if (Position.X >= Size || Position.Y >= Size || Position.X < 0 || Position.Y < 0 || Position.Z < 0) return true;
	if (Position.Z >= Size) return true;
	return Blocks[GetBlockIndex(Position.X, Position.Y, Position.Z)] == EVoxelTutorialBlock::Air;
}

void ANaiveChunk::CreateFace(const EVoxelTutorialDirection Direction, const FVector Position)
{
	const auto Color = FColor::MakeRandomColor();
	const auto Normal = GetNormal(Direction);
	
	MeshData.Vertices.Append(GetFaceVertices(Direction, Position));
	MeshData.Triangles.Append({ VertexCount + 3, VertexCount + 2, VertexCount, VertexCount + 2, VertexCount + 1, VertexCount });
	MeshData.Normals.Append({Normal, Normal, Normal, Normal});
	MeshData.Colors.Append({Color, Color, Color, Color});
	MeshData.UV0.Append({ FVector2D(1,1), FVector2D(1,0), FVector2D(0,0), FVector2D(0,1) });

	VertexCount += 4;
}

TArray<FVector> ANaiveChunk::GetFaceVertices(EVoxelTutorialDirection Direction, const FVector Position) const
{
	TArray<FVector> Vertices;

	for (int i = 0; i < 4; i++)
	{
		Vertices.Add(BlockVertexData[BlockTriangleData[i + static_cast<int>(Direction) * 4]] + Position);
	}
	
	return Vertices;
}

FVector ANaiveChunk::GetPositionInDirection(const EVoxelTutorialDirection Direction, const FVector Position) const
{
	switch (Direction)
	{
	case EVoxelTutorialDirection::Forward: return Position + FVector::ForwardVector;
	case EVoxelTutorialDirection::Back: return Position + FVector::BackwardVector;
	case EVoxelTutorialDirection::Left: return Position + FVector::LeftVector;
	case EVoxelTutorialDirection::Right: return Position + FVector::RightVector;
	case EVoxelTutorialDirection::Up: return Position + FVector::UpVector;
	case EVoxelTutorialDirection::Down: return Position + FVector::DownVector;
	default: throw std::invalid_argument("Invalid direction");
	}
}

FVector ANaiveChunk::GetNormal(const EVoxelTutorialDirection Direction) const
{
	switch (Direction)
	{
	case EVoxelTutorialDirection::Forward: return FVector::ForwardVector;
	case EVoxelTutorialDirection::Right: return FVector::RightVector;
	case EVoxelTutorialDirection::Back: return FVector::BackwardVector;
	case EVoxelTutorialDirection::Left: return FVector::LeftVector;
	case EVoxelTutorialDirection::Up: return FVector::UpVector;
	case EVoxelTutorialDirection::Down: return FVector::DownVector;
	default: throw std::invalid_argument("Invalid direction");
	}
}

void ANaiveChunk::ModifyVoxelData(const FIntVector Position, const EVoxelTutorialBlock Block)
{
	const int Index = GetBlockIndex(Position.X, Position.Y, Position.Z);
	
	Blocks[Index] = Block;
}

int ANaiveChunk::GetBlockIndex(const int X, const int Y, const int Z) const
{
	return Z * Size * Size + Y * Size + X;
}

