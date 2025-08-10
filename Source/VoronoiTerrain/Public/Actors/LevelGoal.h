// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGoal.generated.h"

class USphereComponent;

UCLASS()
class VORONOITERRAIN_API ALevelGoal : public AActor
{
    GENERATED_BODY()

public:
    ALevelGoal();

    UFUNCTION()
    void OnBeginOverlapComponentEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintImplementableEvent, Category = "Goal")
    void OnPlayerReachedGoal();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Goal")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Goal")
    TObjectPtr<USphereComponent> ColliderComponent;

    UFUNCTION(CallInEditor, Category = "Collision")
    void UpdateCollisionToFitMesh();

    virtual void OnConstruction(const FTransform& Transform) override;
};