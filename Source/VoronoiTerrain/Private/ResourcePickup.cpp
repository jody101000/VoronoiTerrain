#include "ResourcePickup.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "NiagaraFunctionLibrary.h"

// Sets default values
AResourcePickup::AResourcePickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ColliderComponent = CreateDefaultSubobject<USphereComponent>("ColliderComponent");
	SetRootComponent(ColliderComponent);
	ColliderComponent->SetGenerateOverlapEvents(true);
	ColliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ColliderComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	ColliderComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ColliderComponent->OnComponentBeginOverlap.AddDynamic(
		this, &AResourcePickup::OnBeginOverlapComponentEvent
	);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetupAttachment(ColliderComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetGenerateOverlapEvents(false);

	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>("RotatingMovementComponent");
}

void AResourcePickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateCollisionToFitMesh();
}

void AResourcePickup::UpdateCollisionToFitMesh()
{
	if (MeshComponent && MeshComponent->GetStaticMesh() && ColliderComponent)
	{
		FBoxSphereBounds MeshBounds = MeshComponent->GetStaticMesh()->GetBounds();
		float SphereRadius = MeshBounds.SphereRadius;
		ColliderComponent->SetSphereRadius(SphereRadius);
	}
}

void AResourcePickup::OnBeginOverlapComponentEvent(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult
)
{
	if (!Cast<ACharacter>(OtherActor)) return;

	if (PickSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, PickSound, GetActorLocation(), VolumeMultiplier);
	}

	if (OnPickupEffect)
	{
		const FVector Offset = GetActorUpVector() * PickEffectSpawnOffset;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, OnPickupEffect, OtherActor->GetActorLocation() + Offset);
	}

	Destroy();
}