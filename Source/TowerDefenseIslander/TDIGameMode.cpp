#include "TDIGameMode.h"
#include "TDIPlayerController.h"
#include "TDITopDownPawn.h"
#include "TDIHUDWidget.h"
#include "Kismet/GameplayStatics.h"

ATDIGameMode::ATDIGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = ATDITopDownPawn::StaticClass();
	PlayerControllerClass = ATDIPlayerController::StaticClass();
}

void ATDIGameMode::BeginPlay()
{
	Super::BeginPlay();
	PlayerGold = StartingGold;
	ChangePhase(EGamePhase::Preparation);
}

void ATDIGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentPhase == EGamePhase::Preparation)
	{
		PhaseTimer -= DeltaTime;
		if (PhaseTimer <= 0.0f)
		{
			StartWave();
		}
	}
}

void ATDIGameMode::StartWave()
{
	CurrentWave++;
	ChangePhase(EGamePhase::Wave);
	OnWaveStarted.Broadcast(CurrentWave);
}

void ATDIGameMode::EndWave()
{
	if (CurrentWave >= TotalWaves)
	{
		ChangePhase(EGamePhase::Victory);
		return;
	}

	AddGold(GoldPerWave);
	ChangePhase(EGamePhase::Preparation);
}

void ATDIGameMode::OnBaseDestroyed()
{
	ChangePhase(EGamePhase::GameOver);
}

bool ATDIGameMode::SpendGold(int32 Amount)
{
	if (PlayerGold < Amount) return false;
	PlayerGold -= Amount;
	OnGoldChanged.Broadcast(PlayerGold);
	return true;
}

void ATDIGameMode::AddGold(int32 Amount)
{
	PlayerGold += Amount;
	OnGoldChanged.Broadcast(PlayerGold);
}

void ATDIGameMode::NotifyEnemyKilled()
{
	if (EnemiesRemainingInWave > 0)
	{
		EnemiesRemainingInWave--;
		if (EnemiesRemainingInWave == 0)
		{
			EndWave();
		}
	}
}

void ATDIGameMode::ChangePhase(EGamePhase NewPhase)
{
	CurrentPhase = NewPhase;
	if (NewPhase == EGamePhase::Preparation)
	{
		PhaseTimer = PreparationTime;
	}
	OnPhaseChanged.Broadcast(NewPhase);
}
