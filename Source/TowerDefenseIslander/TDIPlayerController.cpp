#include "TDIPlayerController.h"
#include "TDITopDownPawn.h"
#include "Towers/TDITowerBase.h"
#include "Territory/TDITerritoryBase.h"
#include "Logistics/TDIRoad.h"
#include "Subsystems/TDITerritorySubsystem.h"
#include "Save/TDISaveManager.h"
#include "UI/TDIHUDBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

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
		if (IA_PrimaryAction)
			EIC->BindAction(IA_PrimaryAction, ETriggerEvent::Started, this,
				&ATDIPlayerController::OnPrimaryAction);

		if (IA_SecondaryAction)
			EIC->BindAction(IA_SecondaryAction, ETriggerEvent::Started, this,
				&ATDIPlayerController::OnSecondaryAction);

		if (IA_PanCamera)
			EIC->BindAction(IA_PanCamera, ETriggerEvent::Triggered, this,
				&ATDIPlayerController::OnPanCamera);

		if (IA_ZoomCamera)
			EIC->BindAction(IA_ZoomCamera, ETriggerEvent::Triggered, this,
				&ATDIPlayerController::OnZoomCamera);

		if (IA_QuickSave)
			EIC->BindAction(IA_QuickSave, ETriggerEvent::Started, this,
				&ATDIPlayerController::OnQuickSave);
	}
}

void ATDIPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsPlacingTower) UpdateGhostTower();
}

void ATDIPlayerController::SelectTowerForPlacement(TSubclassOf<ATDITowerBase> TowerClass)
{
	CancelPlacement();
	if (!TowerClass) return;

	bIsPlacingTower = true;
	PendingTowerClass = TowerClass;

	// Spawn ghost immediately under cursor
	FVector CursorPos;
	if (GetCursorWorldPosition(CursorPos))
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GhostTower = GetWorld()->SpawnActor<ATDITowerBase>(TowerClass, CursorPos,
			FRotator::ZeroRotator, Params);
		if (GhostTower) GhostTower->SetGhostMode(true);
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

bool ATDIPlayerController::AttemptCaptureTerritory(ATDITerritoryBase* Territory)
{
	UTDITerritorySubsystem* Sub = GetWorld()->GetSubsystem<UTDITerritorySubsystem>();
	return Sub ? Sub->BeginCaptureAttempt(Territory) : false;
}

void ATDIPlayerController::OnPrimaryAction(const FInputActionValue& Value)
{
	if (bIsPlacingTower)
	{
		// Attempt to place tower
		if (!GhostTower || !GhostTower->CanPlaceHere()) return;

		FVector PlacePos;
		if (!GetCursorWorldPosition(PlacePos)) return;

		// Deduct cost
		// (Cost check deferred to tower — it calls ResourceSubsystem internally)
		GhostTower->SetActorLocation(PlacePos);
		GhostTower->OnPlaced();
		GhostTower = nullptr;   // Tower now "lives" — don't destroy it

		bIsPlacingTower = false;
		PendingTowerClass = nullptr;
	}
	else
	{
		HandleSelection();
	}
}

void ATDIPlayerController::OnSecondaryAction(const FInputActionValue& Value)
{
	if (bIsPlacingTower)
	{
		CancelPlacement();
	}
	else
	{
		// Future: open context menu
	}
}

void ATDIPlayerController::OnPanCamera(const FInputActionValue& Value)
{
	if (APawn* P = GetPawn())
	{
		FVector2D Axis = Value.Get<FVector2D>();
		P->AddMovementInput(FVector::ForwardVector, Axis.Y);
		P->AddMovementInput(FVector::RightVector, Axis.X);
	}
}

void ATDIPlayerController::OnZoomCamera(const FInputActionValue& Value)
{
	if (ATDITopDownPawn* P = Cast<ATDITopDownPawn>(GetPawn()))
	{
		P->ZoomCamera(Value.Get<float>());
	}
}

void ATDIPlayerController::OnQuickSave(const FInputActionValue& Value)
{
	if (UTDISaveManager* SaveMgr = GetGameInstance()->GetSubsystem<UTDISaveManager>())
	{
		SaveMgr->SaveGame(0);
	}
}

bool ATDIPlayerController::GetCursorWorldPosition(FVector& OutPos) const
{
	FHitResult Hit;
	if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false, Hit))
	{
		OutPos = Hit.Location;
		OutPos.Z = 0.0f;
		return true;
	}
	return false;
}

void ATDIPlayerController::UpdateGhostTower()
{
	if (!GhostTower) return;
	FVector NewPos;
	if (GetCursorWorldPosition(NewPos))
	{
		GhostTower->SetActorLocation(NewPos);
	}
}

void ATDIPlayerController::HandleSelection()
{
	FHitResult Hit;
	if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false, Hit))
	{
		SelectedActor = nullptr;
		NotifyHUDSelection(nullptr);
		return;
	}

	AActor* HitActor = Hit.GetActor();
	SelectedActor = HitActor;
	NotifyHUDSelection(HitActor);
}

void ATDIPlayerController::NotifyHUDSelection(AActor* Actor)
{
	// Find the HUD widget in the viewport
	UUserWidget* HUD = nullptr;
	TArray<UUserWidget*> Widgets;
	// Simpler: broadcast through GameMode or find widget by class
	// For now, direct cast attempt
	if (UTDIHUDBase* TDIHud = Cast<UTDIHUDBase>(HUD))
	{
		if (!Actor) { TDIHud->OnSelectionCleared(); return; }

		if (ATDITowerBase* Tower = Cast<ATDITowerBase>(Actor))
			TDIHud->OnTowerSelected(Tower);
		else if (ATDITerritoryBase* Territory = Cast<ATDITerritoryBase>(Actor))
			TDIHud->OnTerritorySelected(Territory);
		else if (ATDIRoad* Road = Cast<ATDIRoad>(Actor))
			TDIHud->OnRoadSelected(Road);
		else
			TDIHud->OnSelectionCleared();
	}
}
