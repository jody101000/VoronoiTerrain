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
	Super::Tick(DeltaSeconds);

	if (bLeftMouseHold && ClayBuilder)
	{
		ClayBuilder->BrushStrength = 1;
		ClayBuilder->StartBuildClay();
	}
	if (bMiddleMouseHold && ClayBuilder)
	{
		ClayBuilder->BrushStrength = -1;
		ClayBuilder->StartBuildClay();
	}
	ClayBuilder->BrushStrength = 0;
}


void ABuilderPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("LeftMouseButton", IE_Pressed, this, &ABuilderPlayerController::OnLeftMousePressed);
	InputComponent->BindAction("LeftMouseButton", IE_Released, this, &ABuilderPlayerController::OnLeftMouseReleased);
	InputComponent->BindAction("MiddleMouseButton", IE_Pressed, this, &ABuilderPlayerController::OnMiddleMousePressed);
	InputComponent->BindAction("MiddleMouseButton", IE_Released, this, &ABuilderPlayerController::OnMiddleMouseReleased);
}

void ABuilderPlayerController::OnLeftMousePressed()
{
	UE_LOG(LogTemp, Warning, TEXT("BuilderPlayerController: Mouse Press Detected"));
	bLeftMouseHold = true;
}

void ABuilderPlayerController::OnLeftMouseReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("BuilderPlayerController: Mouse Release Detected"));
	bLeftMouseHold = false;
}

void ABuilderPlayerController::OnMiddleMousePressed()
{
	UE_LOG(LogTemp, Warning, TEXT("BuilderPlayerController: Mouse Press Detected"));
	bMiddleMouseHold = true;
}

void ABuilderPlayerController::OnMiddleMouseReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("BuilderPlayerController: Mouse Release Detected"));
	bMiddleMouseHold = false;
}

