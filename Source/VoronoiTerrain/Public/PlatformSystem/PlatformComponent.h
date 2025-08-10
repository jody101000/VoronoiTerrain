#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformPropertyManager.h"

#include "PlatformComponent.generated.h"

class UStaticMeshComponent;

UCLASS()
class VORONOITERRAIN_API APlatformComponent : public AActor
{
    GENERATED_BODY()

public:
    // Constructor
    APlatformComponent();

    // Mesh setting for collision detection
    void PreInitializePlatform(EPlatformType InType, UStaticMesh* InMesh, int InIndex);
    // Transformation initialization
    void PostInitializePlatform(const FVector& Position, const FRotator& Rotation, float Scale);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
    EPlatformType PlatformType;

    UPROPERTY()
    FPlatformAllProperties PlatformProperties;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // Movement updates
    void UpdateMovement(float DeltaTime);
    void UpdateRotation(float DeltaTime);

    UPROPERTY()
    int PlatformIndex;

    // Movement state
    UPROPERTY()
    FVector InitialPosition;

    UPROPERTY()
    FRotator InitialRotation;

    UPROPERTY()
    float MovementTime;
};