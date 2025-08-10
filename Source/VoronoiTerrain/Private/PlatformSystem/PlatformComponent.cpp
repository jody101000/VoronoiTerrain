#include "PlatformSystem/PlatformComponent.h"

#include "../VoronoiTerrainCharacter.h"
#include "PlatformSystem/PlatformTypeManager.h"

#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

APlatformComponent::APlatformComponent()
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    RootComponent = MeshComponent;

    // Setup collision
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    PlatformType = EPlatformType::Standard;
    PlatformIndex = -1;
    MovementTime = 0.0f;
}

void APlatformComponent::BeginPlay()
{
    Super::BeginPlay();

    InitialPosition = GetActorLocation();
    InitialRotation = GetActorRotation();	
}

void APlatformComponent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    MovementTime += DeltaTime;

    UpdateMovement(DeltaTime);
    UpdateRotation(DeltaTime);
}

void APlatformComponent::PreInitializePlatform(EPlatformType InType, UStaticMesh* InMesh, int InIndex)
{
    PlatformType = InType;
    PlatformIndex = InIndex;

    if (InMesh && MeshComponent)
    {
        MeshComponent->SetStaticMesh(InMesh);
    }
}

void APlatformComponent::PostInitializePlatform(const FVector& Position, const FRotator& Rotation, float Scale)
{
    SetActorLocation(Position);
    SetActorRotation(Rotation);
    SetActorScale3D(FVector(Scale));

    InitialPosition = Position;
    InitialRotation = Rotation;

    UPlatformTypeManager* TypeManager = NewObject<UPlatformTypeManager>();
    PlatformProperties = TypeManager->GetPlatformTypeProperties(PlatformType);
}

void APlatformComponent::UpdateMovement(float DeltaTime)
{
    if (PlatformProperties.MovementProperties.MovementPattern == EMovementPattern::Linear)
    {
        float Phase = MovementTime * PlatformProperties.MovementProperties.MovementSpeed / 100.0f;
        Phase += PlatformProperties.MovementProperties.PhaseOffset * 2.0f * PI;

        float SinValue = FMath::Sin(Phase);
        FVector Direction = InitialPosition;
        Direction.Z = 0;
        Direction.Normalize();
        FVector Offset = Direction * PlatformProperties.MovementProperties.MovementRange * SinValue;

        SetActorLocation(InitialPosition + Offset);
    }
}

void APlatformComponent::UpdateRotation(float DeltaTime)
{
    FRotator RotationDelta = PlatformProperties.MovementProperties.RotationSpeed * DeltaTime;
    if (!RotationDelta.IsNearlyZero())
    {
        FRotator NewRotation = GetActorRotation() + RotationDelta;
        SetActorRotation(NewRotation);
    }
}