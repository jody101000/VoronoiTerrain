#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BrushPreview.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class VORONOITERRAIN_API ABrushPreview : public AActor
{
    GENERATED_BODY()

public:
    ABrushPreview();

    virtual void BeginPlay() override;

    /** Set world-space center and radius (in cm) */
    void SetPreviewTransform(const FVector& Location, float Radius);

    /** Set color and opacity */
    void SetColorAndOpacity(const FLinearColor& Color, float Opacity);

    /** Show / hide */
    void SetVisible(bool bVisible);

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* SphereMeshComp;

    UPROPERTY()
    UMaterialInstanceDynamic* DynMaterial;
};
