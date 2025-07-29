// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformTypeManager.h"

UPlatformTypeManager::UPlatformTypeManager()
{
	PlatformTypeTemplates.Empty();

	PlatformTypeTemplates.Add(EPlatformType::Standard, CreateStandardPlatformProperties());
	PlatformTypeTemplates.Add(EPlatformType::Bounce, CreateBouncePlatformProperties());
	PlatformTypeTemplates.Add(EPlatformType::Rotating, CreateRotatingPlatformProperties());
	PlatformTypeTemplates.Add(EPlatformType::Slippery, CreateSlipperyPlatformProperties());
	PlatformTypeTemplates.Add(EPlatformType::Moving, CreateMovingPlatformProperties());
}

TArray<EPlatformType> UPlatformTypeManager::GetAllPlatformTypes() const
{
	TArray<EPlatformType> Types;
	PlatformTypeTemplates.GetKeys(Types);
	return Types;
}

TArray<EPlatformType> UPlatformTypeManager::GetTypesForDifficulty(int32 TargetDifficulty, int32 Tolerance) const
{
	TArray<EPlatformType> SuitableTypes;

	for (const auto& Type : PlatformTypeTemplates)
	{
		if (Type.Value.DifficultyCheck(TargetDifficulty, Tolerance))
		{
			SuitableTypes.Add(Type.Key);
		}
	}
	return SuitableTypes;
}

FPlatformAllProperties UPlatformTypeManager::GetPlatformTypeProperties(EPlatformType PlatformType) const
{
	if (const FPlatformAllProperties* Properties = PlatformTypeTemplates.Find(PlatformType))
	{
		return *Properties;
	}
	return CreateStandardPlatformProperties();
}

void UPlatformTypeManager::UpdatePlatformProperties(EPlatformType PlatformType, const FPlatformAllProperties& NewProperties)
{
	PlatformTypeTemplates.Add(PlatformType, NewProperties);
}

FPlatformAllProperties UPlatformTypeManager::CreateStandardPlatformProperties() const
{
    FPlatformAllProperties Properties;
    Properties.DisplayName = TEXT("Standard Platform");
    Properties.PlatformType = EPlatformType::Standard;
    Properties.Difficulty = 1;

    // Standard physics
    Properties.PhysicsProperties.FrictionCoefficient = 0.7f;
    Properties.PhysicsProperties.BounceCoefficient = 0.0f;

    // No movement
    Properties.MovementProperties.MovementPattern = EMovementPattern::Static;

    return Properties;
}

FPlatformAllProperties UPlatformTypeManager::CreateBouncePlatformProperties() const
{
    FPlatformAllProperties Properties;
    Properties.DisplayName = TEXT("Bounce Platform");
    Properties.PlatformType = EPlatformType::Bounce;
    Properties.Difficulty = 3;

    // High bounce physics
    Properties.PhysicsProperties.FrictionCoefficient = 0.5f;
    Properties.PhysicsProperties.BounceCoefficient = 1.5f;

    // No movement
    Properties.MovementProperties.MovementPattern = EMovementPattern::Static;

    // Player jump height
    Properties.InteractionModifiers.Add(TEXT("JumpHeightMultiplier"), 1.5f);

    return Properties;
}

FPlatformAllProperties UPlatformTypeManager::CreateRotatingPlatformProperties() const
{
    FPlatformAllProperties Properties;
    Properties.DisplayName = TEXT("Rotating Platform");
    Properties.PlatformType = EPlatformType::Rotating;
    Properties.Difficulty = 4;

    // Standard physics
    Properties.PhysicsProperties.FrictionCoefficient = 0.6f;
    Properties.PhysicsProperties.BounceCoefficient = 0.0f;

    // Rotation without movement
    Properties.MovementProperties.MovementPattern = EMovementPattern::Static;
    Properties.MovementProperties.RotationSpeed = FRotator(0.0f, 10.0f, 0.0f);

    return Properties;
}

FPlatformAllProperties UPlatformTypeManager::CreateSlipperyPlatformProperties() const
{
    FPlatformAllProperties Properties;
    Properties.DisplayName = TEXT("Slippery Platform");
    Properties.PlatformType = EPlatformType::Slippery;
    Properties.Difficulty = 5;

    // low friction
    Properties.PhysicsProperties.FrictionCoefficient = 0.1f;
    Properties.PhysicsProperties.BounceCoefficient = 0.1f;

    // No movement
    Properties.MovementProperties.MovementPattern = EMovementPattern::Static;

    return Properties;
}

FPlatformAllProperties UPlatformTypeManager::CreateMovingPlatformProperties() const
{
    FPlatformAllProperties Properties;
    Properties.DisplayName = TEXT("Moving Platform");
    Properties.PlatformType = EPlatformType::Moving;
    Properties.Difficulty = 6;

    // Standard physics
    Properties.PhysicsProperties.FrictionCoefficient = 0.7f;
    Properties.PhysicsProperties.BounceCoefficient = 0.0f;

    // Linear movement
    Properties.MovementProperties.MovementPattern = EMovementPattern::Linear;
    Properties.MovementProperties.MovementSpeed = 150.0f;
    Properties.MovementProperties.MovementRange = 300.0f;
    Properties.MovementProperties.MovementDirection = FVector(1.0f, 0.0f, 0.0f);

    return Properties;
}