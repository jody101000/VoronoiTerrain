// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BuilderPlayerController.generated.h"

class UClayBuilder;
/**
 * 
 */
UCLASS()
class VORONOITERRAIN_API ABuilderPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABuilderPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupInputComponent() override;

	void OnLeftMousePressed();
	void OnLeftMouseReleased();

	bool bLeftMouseHold = false;
	bool bFirstPress = false;
	
	UPROPERTY()
	UClayBuilder* ClayBuilder;
};
