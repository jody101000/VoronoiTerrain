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

	SetupPlatformCollision();

}

void UMovingPlatformComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	TargetRotation += RotationUpdate;
	TargetRotation = TargetRotation.Clamp();

	TargetPosition += PositionUpdate;
	if (TargetPosition.X >= StartPosition.X + PositionUpdateDist)
	{
		TargetPosition.X = StartPosition.X + PositionUpdateDist;
		PositionUpdate = -PositionUpdate;
	}
	if (TargetPosition.X <= StartPosition.X - PositionUpdateDist)
	{
		TargetPosition.X = StartPosition.X - PositionUpdateDist;
		PositionUpdate = -PositionUpdate;
	}
	

	// move to target position
	SetWorldLocation(TargetPosition);

	// scaling
	SetWorldScale3D(FVector(TargetScale, TargetScale, TargetScale));

	// rotation
	SetWorldRotation(TargetRotation);
}

void UMovingPlatformComponent::InitializePlatform(int InPlatformIndex, const FVector& InitialPosition, const FRotator& InitialRotation, float InitialScale)
{
	PlatformIndex = InPlatformIndex;
	TargetPosition = InitialPosition;
	TargetScale = InitialScale;
	TargetRotation = InitialRotation;

	StartPosition = InitialPosition;
	
	SetWorldLocation(InitialPosition);
	SetWorldScale3D(FVector(InitialScale, InitialScale, InitialScale));
	SetWorldRotation(InitialRotation);
	
	UpdateBounds();
	MarkRenderStateDirty();

	// UE_LOG(LogTemp, Log, TEXT("Platform %d initialized at (%f, %f, %f) with scale %f"),
	// 	PlatformIndex, InitialPosition.X, InitialPosition.Y, InitialPosition.Z, InitialScale);
}

void UMovingPlatformComponent::UpdatePlatformData(const FVector& NewPosition, const FRotator& NewRotation, float NewScale)
{
	TargetPosition = NewPosition;
	TargetScale = NewScale;
	TargetRotation = NewRotation;
}

void UMovingPlatformComponent::SetPositionUpdate(const FVector& Velocity, float Distance)
{
	PositionUpdate = Velocity;
	PositionUpdateDist = Distance;
}


void UMovingPlatformComponent::UpdatePlatformScale(float NewScale)
{
	TargetScale = NewScale;
}


void UMovingPlatformComponent::SetRotationUpdate(const FRotator& Velocity)
{
	RotationUpdate = Velocity;

	// UE_LOG(LogTemp, Warning, TEXT("Rotation Updated to (%f, %f, %f)"), TargetRotation.Pitch, TargetRotation.Yaw, TargetRotation.Roll);
}



void UMovingPlatformComponent::SetupPlatformCollision()
{
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Set collision object type
	SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

	// // collision responses
	// SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	// SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	// SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
	SetSimulatePhysics(false);
	SetNotifyRigidBodyCollision(true);
}

