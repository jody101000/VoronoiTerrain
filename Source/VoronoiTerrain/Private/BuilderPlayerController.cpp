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

	if (ClayBuilder)
	{
		FVector MouseWorldPosition;
		if (ClayBuilder->GetMouseWorldPosition(MouseWorldPosition, 0.5f))
		{
			float CurrentBrushRadius = ClayBuilder->VoxelWorld ? ClayBuilder->VoxelWorld->BrushRadius : 90.0f;
			DrawDebugSphere(GetWorld(), MouseWorldPosition, CurrentBrushRadius, 12, FColor::Yellow, false, -1, 0, 2.0f);
		}
	}

	if ((bLeftMouseHold) && ClayBuilder)
	{
		// Shift pressed - Erasing mode
		bool bShiftPressed = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
		float StrengthMultiplier = bShiftPressed ? -1.0f : 1.0f;
		ClayBuilder->StartBuildClay(StrengthMultiplier);
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
	UE_LOG(LogTemp, Warning, TEXT("BuilderPlayerController: Mouse Press Detected"));
	bLeftMouseHold = true;
}

void ABuilderPlayerController::OnLeftMouseReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("BuilderPlayerController: Mouse Release Detected"));
	bLeftMouseHold = false;
}
