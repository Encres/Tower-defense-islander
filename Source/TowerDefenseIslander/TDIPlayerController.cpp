#include "TDIPlayerController.h"
#include "TDITowerBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/World.h"

ATDIPlayerController::ATDIPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ATDIPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ATDIPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_PlaceTower)
			EIC->BindAction(IA_PlaceTower, ETriggerEvent::Triggered, this, &ATDIPlayerController::OnPlaceTowerInput);

		if (IA_CancelAction)
			EIC->BindAction(IA_CancelAction, ETriggerEvent::Triggered, this, &ATDIPlayerController::OnCancelInput);

		if (IA_ScrollCamera)
			EIC->BindAction(IA_ScrollCamera, ETriggerEvent::Triggered, this, &ATDIPlayerController::OnScrollCameraInput);

		if (IA_PanCamera)
			EIC->BindAction(IA_PanCamera, ETriggerEvent::Triggered, this, &ATDIPlayerController::OnPanCameraInput);
	}
}

void ATDIPlayerController::SelectTowerForPlacement(TSubclassOf<ATDITowerBase> TowerClass)
{
	CancelPlacement();
	if (!TowerClass) return;

	bIsPlacingTower = true;
	PendingTowerClass = TowerClass;

	FVector SpawnLocation;
	if (GetGroundPositionUnderCursor(SpawnLocation))
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GhostTower = GetWorld()->SpawnActor<ATDITowerBase>(TowerClass, SpawnLocation, FRotator::ZeroRotator, Params);
		if (GhostTower)
		{
			GhostTower->SetGhostMode(true);
		}
	}
}

void ATDIPlayerController::CancelPlacement()
{
	bIsPlacingTower = false;
	PendingTowerClass = nullptr;
	if (GhostTower)
	{
		GhostTower->Destroy();
		GhostTower = nullptr;
	}
}

void ATDIPlayerController::OnPlaceTowerInput(const FInputActionValue& Value)
{
	if (!bIsPlacingTower || !GhostTower) return;

	FVector PlaceLocation;
	if (!GetGroundPositionUnderCursor(PlaceLocation)) return;

	if (!GhostTower->CanPlaceHere()) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	ATDITowerBase* NewTower = GetWorld()->SpawnActor<ATDITowerBase>(PendingTowerClass, PlaceLocation, FRotator::ZeroRotator, Params);
	if (NewTower)
	{
		NewTower->OnPlaced();
	}

	CancelPlacement();
}

void ATDIPlayerController::OnCancelInput(const FInputActionValue& Value)
{
	CancelPlacement();
}

void ATDIPlayerController::OnScrollCameraInput(const FInputActionValue& Value)
{
	// Zoom is handled on the pawn's spring arm
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(FVector::UpVector, Value.Get<float>());
	}
}

void ATDIPlayerController::OnPanCameraInput(const FInputActionValue& Value)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		FVector2D PanAxis = Value.Get<FVector2D>();
		ControlledPawn->AddMovementInput(FVector::ForwardVector, PanAxis.Y);
		ControlledPawn->AddMovementInput(FVector::RightVector, PanAxis.X);
	}
}

bool ATDIPlayerController::GetGroundPositionUnderCursor(FVector& OutPosition) const
{
	FHitResult HitResult;
	if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), false, HitResult))
	{
		OutPosition = HitResult.Location;
		OutPosition.Z = 0.0f;
		return true;
	}
	return false;
}

void ATDIPlayerController::UpdateGhostTowerPosition()
{
	if (!GhostTower) return;
	FVector NewPos;
	if (GetGroundPositionUnderCursor(NewPos))
	{
		GhostTower->SetActorLocation(NewPos);
	}
}
