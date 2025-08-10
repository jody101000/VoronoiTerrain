// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformSystem/PlatformTypeManager.h"

UPlatformTypeManager::UPlatformTypeManager()
{
	PlatformTypeTemplates.Empty();

	PlatformTypeTemplates.Add(EPlatformType::Standard, CreateStandardPlatformProperties());
	PlatformTypeTemplates.Add(EPlatformType::Moving, CreateMovingPlatformProperties());
}

TArray<EPlatformType> UPlatformTypeManager::GetAllPlatformTypes() const
{
	TArray<EPlatformType> Types;
	PlatformTypeTemplates.GetKeys(Types);
	return Types;
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

    // Linear movement
    Properties.MovementProperties.MovementPattern = EMovementPattern::Linear;
    Properties.MovementProperties.MovementSpeed = 120.0f;
    Properties.MovementProperties.MovementRange = 300.0f;
    Properties.MovementProperties.MovementDirection = FVector(1.0f, 1.0f, 0.0f);

    return Properties;
}