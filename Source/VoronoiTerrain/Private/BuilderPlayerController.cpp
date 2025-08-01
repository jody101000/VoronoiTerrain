// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderPlayerController.h"
#include "ClayBuilder.h"

ABuilderPlayerController::ABuilderPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ABuilderPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	if (APawn* ControlledPawn = GetPawn())
	{
		ClayBuilder = ControlledPawn->FindComponentByClass<UClayBuilder>();
	}
}

void ABuilderPlayerController::Tick(float DeltaSeconds)
{
	if (bLeftMouseHold)
	{
		ClayBuilder->StartBuildClay();
	}
}


void ABuilderPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("LeftMouseButton", IE_Pressed, this, &ABuilderPlayerController::OnLeftMousePressed);
	InputComponent->BindAction("LeftMouseButton", IE_Released, this, &ABuilderPlayerController::OnLeftMouseReleased);
}

void ABuilderPlayerController::OnLeftMousePressed()
{
	bLeftMouseHold = true;
	UE_LOG(LogTemp, Warning, TEXT("Mouse Press Detected"));
}

void ABuilderPlayerController::OnLeftMouseReleased()
{
	bLeftMouseHold = false;
	UE_LOG(LogTemp, Warning, TEXT("Mouse Release Detected"));
}


