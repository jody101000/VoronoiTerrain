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

	bool bShiftPressed = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
	bool bAltPressed = IsInputKeyDown(EKeys::LeftAlt) || IsInputKeyDown(EKeys::RightAlt);

	if (ClayBuilder)
	{
		FVector MouseWorldPosition;
		float CurrentBrushRadius = ClayBuilder->VoxelWorld ? ClayBuilder->VoxelWorld->BrushRadius : 90.0f;
		if (ClayBuilder->GetMouseWorldPosition(MouseWorldPosition, ECursorActionType::Debug))
		{
			if (bShiftPressed)
			{
				// DrawDebugBox(GetWorld(), MouseWorldPosition, FVector(CurrentBrushRadius / 2), FColor::Red, false, -1, 0, 2.0f);
				DrawDebugSphere(GetWorld(), MouseWorldPosition, CurrentBrushRadius, 12, FColor::Red, false, -1, 0, 2.0f);
			}
			else
			{
				// DrawDebugBox(GetWorld(), MouseWorldPosition, FVector(CurrentBrushRadius / 2), FColor::White, false, -1, 0, 2.0f);
				DrawDebugSphere(GetWorld(), MouseWorldPosition, CurrentBrushRadius, 12, FColor::White, false, -1, 0, 2.0f);
			}
		}
		
		if (bLeftMouseHold && (bAltPressed || bShiftPressed))
		{
			ECursorActionType CursorAction = bShiftPressed ? ECursorActionType::Erase : ECursorActionType::Sculpt;
			ClayBuilder->StartBuildClay(CursorAction);
		}
		
	}

}


void ABuilderPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("LeftMouseButton", IE_Pressed, this, &ABuilderPlayerController::OnLeftMousePressed);
	InputComponent->BindAction("LeftMouseButton", IE_Released, this, &ABuilderPlayerController::OnLeftMouseReleased);
	InputComponent->BindAction("MouseScrollUp", IE_Pressed, this, &ABuilderPlayerController::OnMouseScrollUp);
	InputComponent->BindAction("MouseScrollDown", IE_Pressed, this, &ABuilderPlayerController::OnMouseScrollDown);
}

void ABuilderPlayerController::OnLeftMousePressed()
{
	// UE_LOG(LogTemp, Warning, TEXT("BuilderPlayerController: Mouse Press Detected"));
	bLeftMouseHold = true;
	if (ClayBuilder)
	{
		bool bShiftPressed = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
		ECursorActionType CursorAction = bShiftPressed ? ECursorActionType::Erase : ECursorActionType::Sculpt;
		ClayBuilder->StartBuildClay(CursorAction);

	}
}

void ABuilderPlayerController::OnLeftMouseReleased()
{
	// UE_LOG(LogTemp, Warning, TEXT("BuilderPlayerController: Mouse Release Detected"));
	bLeftMouseHold = false;
}

void ABuilderPlayerController::OnMouseScrollUp()
{
	if (ClayBuilder)
	{
		ClayBuilder->AdjustBuildDistance(ScrollSensitivity);
	}
}

void ABuilderPlayerController::OnMouseScrollDown()
{
	if (ClayBuilder)
	{
		ClayBuilder->AdjustBuildDistance(-ScrollSensitivity);
	}
}