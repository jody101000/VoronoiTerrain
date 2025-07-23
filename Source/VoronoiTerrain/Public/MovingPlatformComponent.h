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

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void InitializePlatform(int InPlatformIndex, const FVector& InitialPosition, float InitialScale = 1.0f);

	int GetPlatformIndex() const { return PlatformIndex; }
};
