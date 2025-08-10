#include "Player/BrushPreview.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

ABrushPreview::ABrushPreview()
{
    PrimaryActorTick.bCanEverTick = false;
    SetActorEnableCollision(false);

    SphereMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
    RootComponent = SphereMeshComp;

    // Load engine sphere mesh (BasicShapes)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshObj(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMeshObj.Succeeded())
    {
        SphereMeshComp->SetStaticMesh(SphereMeshObj.Object);
    }

    SphereMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SphereMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereMeshComp->SetCastShadow(false);
    SphereMeshComp->SetMobility(EComponentMobility::Movable);
    SphereMeshComp->bRenderCustomDepth = false;

    // Load your translucent material you created in the editor
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/Materials/M_BrushPreview.M_BrushPreview"));
    if (Mat.Succeeded())
    {
        SphereMeshComp->SetMaterial(0, Mat.Object);
    }

    // Start hidden
    SetActorHiddenInGame(true);
}

void ABrushPreview::BeginPlay()
{
    Super::BeginPlay();

    if (SphereMeshComp && SphereMeshComp->GetMaterial(0))
    {
        DynMaterial = SphereMeshComp->CreateDynamicMaterialInstance(0);
    }
}

void ABrushPreview::SetPreviewTransform(const FVector& Location, float Radius)
{
    // The builtin sphere mesh has a default radius ~50cm depending on import; we scale uniformly to match desired radius.
    const float MeshRadius = 50.0f; // adjust if your sphere asset differs
    const float Scale = Radius / MeshRadius;

    SetActorLocation(Location);
    SetActorScale3D(FVector(Scale));
}

void ABrushPreview::SetColorAndOpacity(const FLinearColor& Color, float Opacity)
{
    if (DynMaterial)
    {
        DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
        DynMaterial->SetScalarParameterValue(TEXT("Opacity"), Opacity);
    }
}

void ABrushPreview::SetVisible(bool bVisible)
{
    SetActorHiddenInGame(!bVisible);
    SphereMeshComp->SetVisibility(bVisible, true);
}
