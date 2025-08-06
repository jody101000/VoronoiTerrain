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
	
	if (ClayBuilder)
	{
		FVector MouseWorldPosition;
		float CurrentBrushRadius = ClayBuilder->VoxelWorld ? ClayBuilder->VoxelWorld->BrushRadius : 90.0f;
		if (ClayBuilder->GetMouseWorldPosition(MouseWorldPosition, Debug))
		{
			if (bShiftPressed)
			{
				DrawDebugSphere(GetWorld(), MouseWorldPosition, CurrentBrushRadius, 12, FColor::Red, false, -1, 0, 2.0f);
			}
			else
			{
				
				DrawDebugSphere(GetWorld(), MouseWorldPosition, CurrentBrushRadius, 12, FColor::White, false, -1, 0, 2.0f);
			}
		}
		
		if (bLeftMouseHold)
		{
			ECursorActionType CursorAction = bShiftPressed ? Erase : Sculpt;
			ClayBuilder->StartBuildClay(CursorAction);
		}
		
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
