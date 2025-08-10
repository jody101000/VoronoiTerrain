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

protected:
    UPROPERTY(EditAnywhere, Category = "Configuration")
    TMap<EPlatformType, FPlatformAllProperties> PlatformTypeTemplates;

private:

    FPlatformAllProperties CreateStandardPlatformProperties() const;
    FPlatformAllProperties CreateMovingPlatformProperties() const;

};