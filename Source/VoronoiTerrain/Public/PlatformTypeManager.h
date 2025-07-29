// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlatformPropertyManager.h"
#include "PlatformTypeManager.generated.h"

/**
 * Platform Types with Default Properties
 */
UCLASS()
class VORONOITERRAIN_API UPlatformTypeManager : public UObject
{
	GENERATED_BODY()
	
public:
    UPlatformTypeManager();
    
    TArray<EPlatformType> GetAllPlatformTypes() const;
    TArray<EPlatformType> GetTypesForDifficulty(int32 TargetDifficulty, int32 Tolerance = 2) const;
    FPlatformAllProperties GetPlatformTypeProperties(EPlatformType PlatformType) const;
    void UpdatePlatformProperties(EPlatformType PlatformType, const FPlatformAllProperties& NewProperties);

    // // Platform Type - Mesh Mapping//
    // EPlatformType GetPlatformTypeFromMesh(UStaticMesh* StaticMesh) const;
    // UStaticMesh* GetRandomMeshForType(EPlatformType PlatformType, int32 RandomSeed = 0) const;
    // TArray<UStaticMesh*> GetMeshesForType(EPlatformType PlatformType) const;
    // void RegisterMeshForType(UStaticMesh* StaticMesh, EPlatformType PlatformType, float SelectionWeight = 1.0f);
    // void UnregisterMesh(UStaticMesh* StaticMesh);
    // bool HasMeshesForType(EPlatformType PlatformType) const;
    // bool ValidateConfiguration(TArray<FString>& OutErrors) const;

protected:
    UPROPERTY(EditAnywhere, Category = "Configuration")
    TMap<EPlatformType, FPlatformAllProperties> PlatformTypeTemplates;

    // // Mesh to platform type mappings
    // UPROPERTY(EditAnywhere, Category = "Configuration")
    // TArray<FPlatformMeshMapping> MeshMappings;
private:

    FPlatformAllProperties CreateStandardPlatformProperties() const;
    FPlatformAllProperties CreateBouncePlatformProperties() const;
    FPlatformAllProperties CreateRotatingPlatformProperties() const;
    FPlatformAllProperties CreateSlipperyPlatformProperties() const;
    FPlatformAllProperties CreateMovingPlatformProperties() const;

    // // Internal helper to get mesh mappings for a specific type
    // TArray<FPlatformMeshMapping> GetMeshMappingsForType(EPlatformType PlatformType) const;
};