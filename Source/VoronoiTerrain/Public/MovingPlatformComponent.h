#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "MovingPlatformComponent.generated.h"

/**
 * 
 */
UCLASS()
class VORONOITERRAIN_API UMovingPlatformComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UMovingPlatformComponent();

protected:
	virtual void BeginPlay() override;
	
	void SetupPlatformCollision();

	UPROPERTY()
	int PlatformIndex;

	UPROPERTY()
	FVector TargetPosition;

	UPROPERTY()
	float TargetScale;

	UPROPERTY()
	FRotator TargetRotation;

	UPROPERTY()
	FVector StartPosition;

	UPROPERTY(EditAnywhere, Category = "Movement")
	FRotator RotationUpdate = FRotator(0,0,0);

	UPROPERTY(EditAnywhere, Category = "Movement")
	FVector PositionUpdate = FVector(0,0,0);

	UPROPERTY(EditAnywhere, Category = "Movement")
	float PositionUpdateDist = 0;;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void InitializePlatform(int InPlatformIndex, const FVector& InitialPosition, const FRotator& InitialRotation, float InitialScale = 1.0f);
	void UpdatePlatformData(const FVector& NewPosition, const FRotator& NewRotation, float NewScale);
	void SetPositionUpdate(const FVector& Velocity, float Distance);
	void UpdatePlatformScale(float NewScale);
	void SetRotationUpdate(const FRotator& Velocity);

	int GetPlatformIndex() const { return PlatformIndex; }
};
