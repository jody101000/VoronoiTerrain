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

	//  Setup ClayBuilder
	if (APawn* ControlledPawn = GetPawn())
	{
		ClayBuilder = ControlledPawn->FindComponentByClass<UClayBuilder>();
	}

	// Setup brush preview
	if (GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		BrushPreview = GetWorld()->SpawnActor<ABrushPreview>(ABrushPreview::StaticClass(), 
			FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (BrushPreview)
		{
			BrushPreview->SetVisible(false);
		}
	}
}

void ABuilderPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Disable brush preview and do nothing if the mouse is outside viewport
	if (!DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection) || bMouseOverWidget)
	{
		if (BrushPreview)
		{
			BrushPreview->SetVisible(false);
		}
		return;
	}

	const bool bShiftPressed = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);	// Erase
	const bool bAltPressed = IsInputKeyDown(EKeys::LeftAlt) || IsInputKeyDown(EKeys::RightAlt);			// Continuous sculpt

	if (ClayBuilder && ClayBuilder->VoxelWorld)		// ClayBuilder should be set in the Player Character; VoxelWorld should be added to level
	{
		// Set and show brush preview
		FVector BrushWorldLocation;
		if (BrushPreview && ClayBuilder->GetBrushWorldLocation(MouseWorldLocation, MouseWorldDirection, ECursorActionType::Debug, BrushWorldLocation))
		{
			const float CurrentBrushRadius = ClayBuilder->VoxelWorld->BrushRadius;
			BrushPreview->SetPreviewTransform(BrushWorldLocation, CurrentBrushRadius);

			// Preview brush color set base on material using
			const int32 MaterialId = ClayBuilder->VoxelWorld->CurrentMaterialId;
			FLinearColor PreviewColor = FLinearColor::White;
			switch (MaterialId)
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
			const float Opacity = bShiftPressed ? 0.15f : 0.25f;

			BrushPreview->SetColorAndOpacity(PreviewColor, Opacity);
			BrushPreview->SetVisible(true);
		}
		else if (BrushPreview)
		{
			BrushPreview->SetVisible(false);
		}
		
		// Sculpt or erase
		if (bLeftMouseHold && (bAltPressed || bShiftPressed || bSingleClick))
		{
			const ECursorActionType CursorAction = bShiftPressed ? ECursorActionType::Erase : ECursorActionType::Sculpt;
			ClayBuilder->ApplyCursorAction(MouseWorldLocation, MouseWorldDirection, CursorAction);
			bSingleClick = false;
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
	bSingleClick = true;
}

void ABuilderPlayerController::OnLeftMouseReleased()
{
	bLeftMouseHold = false;
}

void ABuilderPlayerController::OnMouseScrollUp()
{
	// Increase brush distance
	if (ClayBuilder)
	{
		ClayBuilder->AdjustBuildDistance(ScrollSensitivity);
	}
}

void ABuilderPlayerController::OnMouseScrollDown()
{
	// Decrease brush distance
	if (ClayBuilder)
	{
		ClayBuilder->AdjustBuildDistance(-ScrollSensitivity);
	}
}