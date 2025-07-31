#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PlatformPropertyManager.h"
#include "Components/BoxComponent.h"
#include "PlatformComponent.generated.h"

USTRUCT()
struct FActorOBB
{
    GENERATED_BODY()
	
    FVector Center;
 
    FVector Forward;
    FVector Right;
    FVector Up;
 
    FActorOBB()
        : Center(0.0f, 0.0f, 0.0f),
          Forward(0.0f, 0.0f, 0.0f),
          Right(0.0f, 0.0f, 0.0f),
          Up(0.0f, 0.0f, 0.0f)
    {
    }
};


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

    UPROPERTY(EditAnywhere)
    UBoxComponent* PlatformTriggerVolume;

    void ApplyPlatformProperties();
    void UpdateMovement(float DeltaTime);
    void UpdateRotation(float DeltaTime);

    FActorOBB GetPlatformOBB(FBox& PlatformAABB);

public:

    void PreInitializePlatform(EPlatformType InType, UStaticMesh* InMesh, int InIndex);

    void PostInitializePlatform(const FVector& Position, const FRotator& Rotation, float Scale);

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

    UFUNCTION()
    void OnPlatformEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};