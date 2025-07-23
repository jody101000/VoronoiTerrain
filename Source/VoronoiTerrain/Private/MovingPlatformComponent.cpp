#include "MovingPlatformComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"

UMovingPlatformComponent::UMovingPlatformComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	PlatformIndex = -1;
	TargetPosition = FVector::ZeroVector;
	TargetScale = 1.0f;
	
	SetupPlatformCollision();
}

void UMovingPlatformComponent::BeginPlay()
{
	Super::BeginPlay();

	// SetupPlatformCollision();

}

void UMovingPlatformComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// move to target position
	if (!TargetPosition.Equals(GetComponentLocation(), 1.0f))
	{
		FVector NewPosition = FMath::VInterpTo(GetComponentLocation(), TargetPosition, DeltaTime, 2.0f);
		SetWorldLocation(NewPosition);
	}

	// scaling
	FVector CurrentScale = GetComponentScale();
	FVector TargetScaleVector(TargetScale, TargetScale, 0.1f);
	if (!CurrentScale.Equals(TargetScaleVector, 0.01f))
	{
		FVector NewScale = FMath::VInterpTo(CurrentScale, TargetScaleVector, DeltaTime, 2.0f);
		SetWorldScale3D(NewScale);
	}
}

void UMovingPlatformComponent::InitializePlatform(int InPlatformIndex, const FVector& InitialPosition, float InitialScale)
{
	PlatformIndex = InPlatformIndex;
	TargetPosition = InitialPosition;
	TargetScale = InitialScale;
	
	SetWorldLocation(InitialPosition);
	SetWorldScale3D(FVector(InitialScale, InitialScale, 0.1f)); // 0.1 for flat cylinder
	
	UpdateBounds();
	MarkRenderStateDirty();

	UE_LOG(LogTemp, Log, TEXT("Platform %d initialized at (%f, %f, %f) with scale %f"),
		PlatformIndex, InitialPosition.X, InitialPosition.Y, InitialPosition.Z, InitialScale);
}

void UMovingPlatformComponent::UpdatePlatformData(const FVector& NewPosition, float NewScale)
{
	TargetPosition = NewPosition;
	TargetScale = NewScale;
}


void UMovingPlatformComponent::SetupPlatformCollision()
{
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	SetSimulatePhysics(false);
	SetNotifyRigidBodyCollision(true);
}

