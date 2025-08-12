#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BuilderPlayerController.generated.h"

class UClayBuilder;
class ABrushPreview;

UCLASS(BlueprintType)
class VORONOITERRAIN_API ABuilderPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABuilderPlayerController();

	UFUNCTION(BlueprintCallable, Category = "Mouse Status")
	bool GetMouseOverWidget(){return bMouseOverWidget;}

	UFUNCTION(BlueprintCallable, Category = "Mouse Status")
	void SetMouseOverWidget(bool OverWidget){ bMouseOverWidget = OverWidget;}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Settings")
	float ScrollSensitivity = 100.0f;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

private:
	// --- Mouse Controls --- //
	void OnLeftMousePressed();
	void OnLeftMouseReleased();
	void OnMouseScrollUp();
	void OnMouseScrollDown();

	bool bSingleClick = false;
	bool bLeftMouseHold = false;
	bool bMouseOverWidget = false;
	FVector MouseWorldLocation, MouseWorldDirection;

	UPROPERTY()
	ABrushPreview* BrushPreview = nullptr;
	
	UPROPERTY()
	UClayBuilder* ClayBuilder = nullptr;
};
