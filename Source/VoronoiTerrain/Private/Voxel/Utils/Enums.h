#pragma once

UENUM(BlueprintType)
enum class EVoxelTutorialDirection : uint8
{
	Forward, Right, Back, Left, Up, Down
};

UENUM(BlueprintType)
enum class EVoxelTutorialBlock : uint8
{
	Null, Air, Stone, Dirt, Grass
};

UENUM(BlueprintType)
enum class EVoxelTutorialGenerationType : uint8
{
	GT_3D UMETA(DisplayName = "3D"),
	GT_2D UMETA(DisplayName = "2D"),
};