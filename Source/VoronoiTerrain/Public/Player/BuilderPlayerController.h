// Fill out your copyright notice in the Description page of Project Settings.

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMouseOverWidget = false;

	UFUNCTION(BlueprintCallable)
	bool GetMouseOverWidget(){return bMouseOverWidget;}

	UFUNCTION(BlueprintCallable)
	void SetMouseOverWidget(bool OverWidget){ bMouseOverWidget = OverWidget;}

	UPROPERTY()
	UClayBuilder* ClayBuilder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Settings")
	float ScrollSensitivity = 100.0f;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

private:
	void OnLeftMousePressed();
	void OnLeftMouseReleased();
	void OnMouseScrollUp();
	void OnMouseScrollDown();

	bool bLeftMouseHold = false;
	ABrushPreview* BrushPreviewActor = nullptr;
};
