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

    // overlap events for interaction
    MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &APlatformComponent::OnPlatformBeginOverlap);
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
        // physical material
    }
}

void APlatformComponent::UpdateMovement(float DeltaTime)
{
    if (PlatformProperties.MovementProperties.MovementPattern == EMovementPattern::Linear)
    {
        float Phase = MovementTime * PlatformProperties.MovementProperties.MovementSpeed / 100.0f;
        Phase += PlatformProperties.MovementProperties.PhaseOffset * 2.0f * PI;

        float SinValue = FMath::Sin(Phase);
        FVector Offset = PlatformProperties.MovementProperties.MovementDirection *
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

void APlatformComponent::OnPlatformBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (AVoronoiTerrainCharacter* Character = Cast<AVoronoiTerrainCharacter>(OtherActor))
    {
        if (Cast<UCapsuleComponent>(OtherComp) == Character->GetCapsuleComponent())
        {
            // Apply platform-specific effects
            if (PlatformType == EPlatformType::Bounce)
            {
                float JumpMultiplier = 1.0f;
                if (float* Modifier = PlatformProperties.InteractionModifiers.Find(TEXT("JumpHeightMultiplier")))
                {
                    JumpMultiplier = *Modifier;
                }

                // upward velocity
                FVector Velocity = Character->GetCharacterMovement()->Velocity;
                Velocity.Z = Character->GetCharacterMovement()->JumpZVelocity * JumpMultiplier;
                Character->GetCharacterMovement()->Velocity = Velocity;
                Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
            }
            else if (PlatformType == EPlatformType::Slippery)
            {
                // reduce friction
                Character->GetCharacterMovement()->GroundFriction =
                    PlatformProperties.PhysicsProperties.FrictionCoefficient;
            }
        }
    }
}