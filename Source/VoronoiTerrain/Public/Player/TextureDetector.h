#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelSystem/VoxelWorld.h"
#include "VoxelSystem/DynamicVoxelChunk.h"
#include "TextureDetector.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTextureDetected, int32, TextureId, FVector, WorldPosition);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class VORONOITERRAIN_API UTextureDetector : public UActorComponent
{
    GENERATED_BODY()

public:
    UTextureDetector();

    UFUNCTION(BlueprintCallable, Category = "Texture Detection")
    int32 DetectTextureAtMousePosition() const;

    UFUNCTION(BlueprintCallable, Category = "Texture Detection")
    int32 DetectTextureAtWorldPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Texture Detection")
    bool GetMouseWorldPositionOnVoxels(FVector& OutWorldPosition) const;

    UPROPERTY(BlueprintAssignable, Category = "Texture Detection")
    FOnTextureDetected OnTextureDetected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    AVoxelWorld* VoxelWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxTraceDistance = 10000.0f;

protected:
    virtual void BeginPlay() override;
};