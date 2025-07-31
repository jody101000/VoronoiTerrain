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

    PlatformTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Platform Trigger Volumne"));
    PlatformTriggerVolume->SetupAttachment(MeshComponent);

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
    // MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &APlatformComponent::OnPlatformBeginOverlap);
    
    PlatformTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformComponent::OnPlatformBeginOverlap);
    PlatformTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &APlatformComponent::OnPlatformEndOverlap);	
}



FActorOBB APlatformComponent::GetPlatformOBB(FBox& PlatformAABB)
{
    const auto Transform = GetTransform();
 
    // Get World space Location.
    const FVector Center = Transform.TransformPosition(PlatformAABB.GetCenter());
 
    // And World space extent
    const FVector Extent = PlatformAABB.GetExtent();
    const FVector Forward = Transform.TransformVector(FVector::ForwardVector * Extent.X);
    const FVector Right = Transform.TransformVector(FVector::RightVector * Extent.Y);
    const FVector Up = Transform.TransformVector(FVector::UpVector * Extent.Z);
 
    // Now you have an oriented bounding box represented by a `Center` and three extent vectors.
    FActorOBB OrientedBox;
    OrientedBox.Center = Center;
    OrientedBox.Forward = Forward;
    OrientedBox.Right = Right;
    OrientedBox.Up = Up;
 
    return OrientedBox;
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
    
    if (MeshComponent->GetStaticMesh())
    {
        FBox PlatformAABB = MeshComponent->GetStaticMesh()->GetBoundingBox();
        PlatformAABB = PlatformAABB.ExpandBy(FVector(0,0,2));
        PlatformTriggerVolume->SetBoxExtent(PlatformAABB.GetExtent());

		// DrawDebugBox(GetWorld(), PlatformTriggerVolume->GetCenterOfMass(), PlatformTriggerVolume->GetScaledBoxExtent(), FColor::Orange, true, -1, 0, 2);
    }

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

                // upward Z velocity
                Character->GetCharacterMovement()->JumpZVelocity *= PlatformProperties.PhysicsProperties.BounceCoefficient;
                UE_LOG(LogTemp, Warning, TEXT("Step On Bounce Platform"));
            }
            else if (PlatformType == EPlatformType::Slippery)
            {
                // reduce friction
                Character->GetCharacterMovement()->BrakingDecelerationWalking = PlatformProperties.PhysicsProperties.BrakingDeceleration;
                UE_LOG(LogTemp, Warning, TEXT("Step On Slippery Platform"));
            }
        }
    }
}

void APlatformComponent::OnPlatformEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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

                // upward Z velocity
                Character->GetCharacterMovement()->JumpZVelocity /= PlatformProperties.PhysicsProperties.BounceCoefficient;
                UE_LOG(LogTemp, Warning, TEXT("Step Off Bounce Platform"));
            }
            else if (PlatformType == EPlatformType::Slippery)
            {
                // reduce friction
                Character->GetCharacterMovement()->BrakingDecelerationWalking = 2000.0;
                UE_LOG(LogTemp, Warning, TEXT("Step Off Slippery Platform"));
            }
        }
    }
}