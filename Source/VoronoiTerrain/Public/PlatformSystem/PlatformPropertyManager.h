#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "PlatformPropertyManager.generated.h"

UENUM()
enum class EResourceType : uint8
{
	None         UMETA(DisplayName = "No Resource"),
	Resource1    UMETA(DisplayName = "Resource Type 1"),
	Resource2    UMETA(DisplayName = "Resource Type 2"),
	Resource3    UMETA(DisplayName = "Resource Type 3")
};

UENUM()
enum class EPlatformType : uint8
{
	Standard        UMETA(DisplayName = "Standard Platform"),
	Moving          UMETA(DisplayName = "Moving Platform")
};

UENUM()
enum class EMovementPattern : uint8
{
	Linear          UMETA(DisplayName = "Linear Movement"),
	Static          UMETA(DisplayName = "No Movement")
};

USTRUCT()
struct VORONOITERRAIN_API FPlatformMovementProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement")
	EMovementPattern MovementPattern = EMovementPattern::Static;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0.0"))
	float MovementSpeed = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0.0"))
	float MovementRange = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	FRotator RotationSpeed = FRotator(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (EditCondition = "MovementPattern == EMovementPattern::Linear"))
	FVector MovementDirection = FVector(1.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PhaseOffset = 0.0f;

	FPlatformMovementProperties()
	{
		MovementPattern = EMovementPattern::Static;
		MovementSpeed = 100.0f;
		MovementRange = 200.0f;
		RotationSpeed = FRotator(0.0f, 0.0f, 0.0f);
		MovementDirection = FVector(1.0f, 0.0f, 0.0f);
		PhaseOffset = 0.0f;
	}
};

USTRUCT()
struct VORONOITERRAIN_API FPlatformAllProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "General")
    FString DisplayName;

    UPROPERTY(EditAnywhere, Category = "General")
    EPlatformType PlatformType = EPlatformType::Standard;

    UPROPERTY(EditAnywhere, Category = "Properties")
    FPlatformMovementProperties MovementProperties;

    UPROPERTY(EditAnywhere, Category = "Difficulty", meta = (ClampMin = "1", ClampMax = "10"))
    int32 Difficulty = 5;
	
    UPROPERTY(EditAnywhere, Category = "Player Interaction")
    TMap<FString, float> InteractionModifiers;

    FPlatformAllProperties()
    {
        DisplayName = TEXT("Standard Platform");
        PlatformType = EPlatformType::Standard;
        MovementProperties = FPlatformMovementProperties();
        Difficulty = 5;
    }

    bool DifficultyCheck(int32 TargetDifficulty, int32 Tolerance = 2) const
    {
        return FMath::Abs(Difficulty - TargetDifficulty) <= Tolerance;
    }
};