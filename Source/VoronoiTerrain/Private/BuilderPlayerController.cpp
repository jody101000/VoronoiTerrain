// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderPlayerController.h"
#include "BrushPreview.h"
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

	if (GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		BrushPreviewActor = GetWorld()->SpawnActor<ABrushPreview>(ABrushPreview::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (BrushPreviewActor)
		{
			BrushPreviewActor->SetVisible(false); // start hidden
		}
	}
}

void ABuilderPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	float MouseX, MouseY;
	if (!GetMousePosition(MouseX, MouseY))
	{
		if (BrushPreviewActor) BrushPreviewActor->SetVisible(false);
		return; // Mouse is outside viewport
	}

	// Check viewport bounds
	if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
	{
		FIntPoint ViewportSize = ViewportClient->Viewport->GetSizeXY();
		if (MouseX < 0 || MouseY < 0 || MouseX >= ViewportSize.X || MouseY >= ViewportSize.Y)
		{
			if (BrushPreviewActor) BrushPreviewActor->SetVisible(false);
			return;
		}
	}

	bool bShiftPressed = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
	bool bAltPressed = IsInputKeyDown(EKeys::LeftAlt) || IsInputKeyDown(EKeys::RightAlt);

	if (ClayBuilder)
	{
		FVector MouseWorldPosition;
		float CurrentBrushRadius = ClayBuilder->VoxelWorld ? ClayBuilder->VoxelWorld->BrushRadius : 90.0f;
		if (ClayBuilder->GetMouseWorldPosition(MouseWorldPosition, ECursorActionType::Debug))
		{
			int32 TextureId = ClayBuilder->GetTextureIdAtCursor();

			if (!bMouseOverWidget)
			{
				if (BrushPreviewActor)
				{
					BrushPreviewActor->SetPreviewTransform(MouseWorldPosition, CurrentBrushRadius);

					FLinearColor PreviewColor = FLinearColor::White;
					switch (TextureId)
					{
					case 1: PreviewColor = FLinearColor::Red; break;
					case 2: PreviewColor = FLinearColor::Green; break;
					case 3: PreviewColor = FLinearColor::Blue; break;
					default: PreviewColor = bShiftPressed ? FLinearColor::Red : FLinearColor::White; break;
					}
					float Opacity = bShiftPressed ? 0.15f : 0.25f;

					BrushPreviewActor->SetColorAndOpacity(PreviewColor, Opacity);

					BrushPreviewActor->SetVisible(true);
				}
			}
			else
			{
				if (BrushPreviewActor) BrushPreviewActor->SetVisible(false);
			}
		}
		else
		{
			if (BrushPreviewActor) BrushPreviewActor->SetVisible(false);
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