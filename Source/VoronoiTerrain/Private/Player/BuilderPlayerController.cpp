#include "Player/BuilderPlayerController.h"
#include "Player/BrushPreview.h"
#include "Player/ClayBuilder.h"
#include "VoxelSystem/VoxelWorld.h"

ABuilderPlayerController::ABuilderPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ABuilderPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* ControlledPawn = GetPawn())
	{
		ClayBuilder = ControlledPawn->FindComponentByClass<UClayBuilder>();
	}

	if (GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		BrushPreviewActor = GetWorld()->SpawnActor<ABrushPreview>(ABrushPreview::StaticClass(), 
			FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (BrushPreviewActor)
		{
			BrushPreviewActor->SetVisible(false);
		}
	}
}

void ABuilderPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Check if mouse is outside viewport
	float MouseX, MouseY;
	if (!GetMousePosition(MouseX, MouseY))
	{
		if (BrushPreviewActor)
		{
			BrushPreviewActor->SetVisible(false);
		}
		return;
	}

	bool bShiftPressed = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
	bool bAltPressed = IsInputKeyDown(EKeys::LeftAlt) || IsInputKeyDown(EKeys::RightAlt);

	if (ClayBuilder)
	{
		FVector MouseWorldPosition;
		float CurrentBrushRadius = ClayBuilder->VoxelWorld ? ClayBuilder->VoxelWorld->BrushRadius : 90.0f;

		// Brush (cursor with radius) preview
		// Only show preview when mouse not on widget
		if (ClayBuilder->GetMouseWorldPosition(MouseWorldPosition, ECursorActionType::Debug))
		{
			int32 TextureId = ClayBuilder->VoxelWorld->CurrentMaterialId;

			if (!bMouseOverWidget)
			{
				if (BrushPreviewActor)
				{
					BrushPreviewActor->SetPreviewTransform(MouseWorldPosition, CurrentBrushRadius);

					FLinearColor PreviewColor = FLinearColor::White;
					switch (TextureId)
					{
						case 1: 
							PreviewColor = FLinearColor::Yellow;
							break;
						case 2:
							PreviewColor = FLinearColor::Green;
							break;
						case 3:
							PreviewColor = FLinearColor::Blue;
							break;
						default:
							PreviewColor = bShiftPressed ? FLinearColor::Black : FLinearColor::White;
							break;
					}
					float Opacity = bShiftPressed ? 0.15f : 0.25f;

					BrushPreviewActor->SetColorAndOpacity(PreviewColor, Opacity);
					BrushPreviewActor->SetVisible(true);
				}
			}
			else if (BrushPreviewActor)
			{
				BrushPreviewActor->SetVisible(false);
			}
		}
		else if (BrushPreviewActor)
		{
			BrushPreviewActor->SetVisible(false);
		}
		
		// Sculpt
		if (bLeftMouseHold && (bAltPressed || bShiftPressed))
		{
			ECursorActionType CursorAction = bShiftPressed ? ECursorActionType::Erase : ECursorActionType::Sculpt;
			ClayBuilder->StartBuildClay(CursorAction);	// ToDo: rename
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
	bLeftMouseHold = true;
	if (ClayBuilder)	// Single click
	{
		bool bShiftPressed = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
		ECursorActionType CursorAction = bShiftPressed ? ECursorActionType::Erase : ECursorActionType::Sculpt;
		ClayBuilder->StartBuildClay(CursorAction);

	}
}

void ABuilderPlayerController::OnLeftMouseReleased()
{
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