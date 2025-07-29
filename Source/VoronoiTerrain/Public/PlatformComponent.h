#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PlatformPropertyManager.h"
#include "PlatformComponent.generated.h"

UCLASS()
class VORONOITERRAIN_API APlatformComponent : public AActor
{
    GENERATED_BODY()

public:
    APlatformComponent();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    UStaticMeshComponent* MeshComponent;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
    EPlatformType PlatformType;

    UPROPERTY()
    FPlatformAllProperties PlatformProperties;

    // Movement state
    UPROPERTY()
    FVector InitialPosition;

    UPROPERTY()
    FRotator InitialRotation;

    UPROPERTY()
    float MovementTime;

    UPROPERTY()
    int PlatformIndex;

    void ApplyPlatformProperties();
    void UpdateMovement(float DeltaTime);
    void UpdateRotation(float DeltaTime);

public:
    void InitializePlatform(EPlatformType InType, UStaticMesh* InMesh, int InIndex,
        const FVector& Position, const FRotator& Rotation, float Scale);

    EPlatformType GetPlatformType() const { return PlatformType; }
    int GetPlatformIndex() const { return PlatformIndex; }

    UFUNCTION(BlueprintCallable, Category = "Platform")
    void SetPlatformType(EPlatformType NewType);

    UFUNCTION()
    void OnPlatformBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};