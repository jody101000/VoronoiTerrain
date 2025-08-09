// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGoal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"


ALevelGoal::ALevelGoal()
{
    ColliderComponent = CreateDefaultSubobject<USphereComponent>("ColliderComponent");
    SetRootComponent(ColliderComponent);
    ColliderComponent->SetGenerateOverlapEvents(true);
    ColliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ColliderComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    ColliderComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    ColliderComponent->OnComponentBeginOverlap.AddDynamic(this, &ALevelGoal::OnBeginOverlapComponentEvent);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
    MeshComponent->SetupAttachment(ColliderComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ALevelGoal::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    UpdateCollisionToFitMesh();
}

void ALevelGoal::UpdateCollisionToFitMesh()
{
    if (MeshComponent && MeshComponent->GetStaticMesh() && ColliderComponent)
    {
        FBoxSphereBounds MeshBounds = MeshComponent->GetStaticMesh()->GetBounds();
        float SphereRadius = MeshBounds.SphereRadius;
        ColliderComponent->SetSphereRadius(SphereRadius);
    }
}

void ALevelGoal::OnBeginOverlapComponentEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (Cast<ACharacter>(OtherActor))
    {
        OnPlayerReachedGoal(); // Blueprint event
    }
}