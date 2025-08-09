#include "PlatformComponent.h"
#include "PlatformTypeManager.h"
#include "../VoronoiTerrainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
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
    MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

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

    // Set mesh for collision detection during spawn
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

    ApplyPlatformProperties();
}

void APlatformComponent::InitializePlatform(EPlatformType InType, UStaticMesh* InMesh, int InIndex,
    const FVector& Position, const FRotator& Rotation, float Scale)
{
    PreInitializePlatform(InType, InMesh, InIndex);
    PostInitializePlatform(Position, Rotation, Scale);
}

void APlatformComponent::SetPlatformType(EPlatformType NewType)
{
    PlatformType = NewType;
    ApplyPlatformProperties();
}

void APlatformComponent::ApplyPlatformProperties()
{
    UPlatformTypeManager* TypeManager = NewObject<UPlatformTypeManager>();
    PlatformProperties = TypeManager->GetPlatformTypeProperties(PlatformType);

    // Apply physics properties
    if (MeshComponent)
    {
        
    }
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
        FVector Offset = Direction *
            PlatformProperties.MovementProperties.MovementRange * SinValue;

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