#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PlatformPropertyManager.h"
#include "Components/BoxComponent.h"
#include "PlatformComponent.generated.h"


UCLASS()
class VORONOITERRAIN_API APlatformComponent : public AActor
{
    GENERATED_BODY()

public:
    APlatformComponent();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
    EPlatformType PlatformType;

    UPROPERTY()
    int PlatformIndex;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY()
    FPlatformAllProperties PlatformProperties;

    // Movement state
    UPROPERTY()
    FVector InitialPosition;

    UPROPERTY()
    FRotator InitialRotation;

    UPROPERTY()
    float MovementTime;

    void ApplyPlatformProperties();
    void UpdateMovement(float DeltaTime);
    void UpdateRotation(float DeltaTime);

public:

    void PreInitializePlatform(EPlatformType InType, UStaticMesh* InMesh, int InIndex);

    void PostInitializePlatform(const FVector& Position, const FRotator& Rotation, float Scale);

    void InitializePlatform(EPlatformType InType, UStaticMesh* InMesh, int InIndex,
        const FVector& Position, const FRotator& Rotation, float Scale);

    EPlatformType GetPlatformType() const { return PlatformType; }
    int GetPlatformIndex() const { return PlatformIndex; }

    UFUNCTION(BlueprintCallable, Category = "Platform")
    void SetPlatformType(EPlatformType NewType);
};