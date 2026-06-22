#include "TDIGameMode.h"
#include "TDIPlayerController.h"
#include "TDITopDownPawn.h"
#include "Castle/TDICastle.h"
#include "Waves/TDIWaveManager.h"
#include "Research/TDIResearchManager.h"
#include "UI/TDIHUDBase.h"
#include "Save/TDISaveManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ATDIGameMode::ATDIGameMode()
{
	DefaultPawnClass = ATDITopDownPawn::StaticClass();
	PlayerControllerClass = ATDIPlayerController::StaticClass();
}

void ATDIGameMode::BeginPlay()
{
	Super::BeginPlay();

	CacheActors();
	CreateHUD();

	// Wire up castle
	if (CachedCastle)
	{
		CachedCastle->OnCastleDestroyed.AddDynamic(this, &ATDIGameMode::OnCastleDestroyed);
	}

	// Wire up wave manager
	if (CachedWaveManager)
	{
		CachedWaveManager->OnWaveStarted.AddDynamic(this, &ATDIGameMode::OnWaveStarted);
		CachedWaveManager->OnWaveCompleted.AddDynamic(this, &ATDIGameMode::OnWaveCompleted);
		CachedWaveManager->OnAllWavesCompleted.AddDynamic(this, &ATDIGameMode::OnAllWavesCompleted);
	}

	// Restore auto-save if it exists
	if (UTDISaveManager* SaveMgr = GetGameInstance()->GetSubsystem<UTDISaveManager>())
	{
		SaveMgr->StartAutoSave(120.0f);  // Auto-save every 2 minutes

		if (SaveMgr->HasPendingSave())
		{
			SaveMgr->ApplySaveToCurrentWorld();
		}
	}

	SetPhase(EGamePhase::Preparation);
}

void ATDIGameMode::SetPhase(EGamePhase NewPhase)
{
	CurrentPhase = NewPhase;
	OnGamePhaseChanged.Broadcast(NewPhase);

	if (HUDWidget)
	{
		HUDWidget->OnPhaseChanged(NewPhase);
	}
}

void ATDIGameMode::OnCastleDestroyed()
{
	SetPhase(EGamePhase::GameOver);

	// Pause all enemies
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), Enemies);
	for (AActor* Enemy : Enemies)
	{
		Enemy->SetActorTickEnabled(false);
	}
}

void ATDIGameMode::OnAllWavesCompleted()
{
	SetPhase(EGamePhase::Victory);
}

void ATDIGameMode::OnWaveStarted(int32 WaveNumber)
{
	SetPhase(EGamePhase::Wave);
}

void ATDIGameMode::OnWaveCompleted(int32 WaveNumber)
{
	SetPhase(EGamePhase::WaveComplete);

	// Brief delay then transition to preparation
	FTimerHandle DelayTimer;
	GetWorldTimerManager().SetTimer(DelayTimer,
		[this]() { if (CurrentPhase == EGamePhase::WaveComplete) SetPhase(EGamePhase::Preparation); },
		3.0f, false);
}

void ATDIGameMode::CacheActors()
{
	CachedCastle = Cast<ATDICastle>(
		UGameplayStatics::GetActorOfClass(this, ATDICastle::StaticClass()));
	CachedWaveManager = Cast<ATDIWaveManager>(
		UGameplayStatics::GetActorOfClass(this, ATDIWaveManager::StaticClass()));
	CachedResearchManager = Cast<ATDIResearchManager>(
		UGameplayStatics::GetActorOfClass(this, ATDIResearchManager::StaticClass()));
}

void ATDIGameMode::CreateHUD()
{
	if (!HUDWidgetClass) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	HUDWidget = CreateWidget<UTDIHUDBase>(PC, HUDWidgetClass);
	if (HUDWidget) HUDWidget->AddToViewport();
}
