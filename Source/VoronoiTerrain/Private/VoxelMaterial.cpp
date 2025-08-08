#include "VoxelMaterial.h"

FLinearColor UVoxelMaterial::Encode(const int Id)
{
	switch (Id)
	{
	case 0:
		return FLinearColor(0, 0, 0, 0);
	case 1:
		return FLinearColor(1, 0, 0, 1);
	case 2:
		return FLinearColor(0, 1, 0, 1);
	case 3:
		return FLinearColor(0, 0, 1, 1);
	default:
		return FLinearColor(0, 0, 0, 0);
	}
}