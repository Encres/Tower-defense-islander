#include "TDIWaveManager.h"
#include "Data/TDIWaveData.h"
#include "Data/TDIEnemyData.h"
#include "Enemies/TDIEnemyBase.h"
#include "Enemies/TDIEnemyAIController.h"
#include "Territory/TDIMonsterSpawner.h"
#include "Subsystems/TDIResourceSubsystem.h"
#include "Kismet/GameplayStatics.h"

ATDIWaveManager::ATDIWaveManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATDIWaveManager::BeginPlay()
{
	Super::BeginPlay();
	// Start the first preparation countdown
	PreparationTimer = PreparationTime;
}

void ATDIWaveManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bWaveActive)
	{
		// Count down preparation timer
		if (PreparationTimer > 0.0f)
		{
			PreparationTimer -= DeltaTime;
			OnPreparationTimerUpdated.Broadcast(FMath::Max(PreparationTimer, 0.0f), PreparationTime);

			if (PreparationTimer <= 0.0f)
			{
				StartNextWave();
			}
		}
		return;
	}

	// Process pending spawn groups
	for (int32 i = PendingGroups.Num() - 1; i >= 0; --i)
	{
		FPendingGroup& Pending = PendingGroups[i];

		Pending.DelayRemaining -= DeltaTime;
		if (Pending.DelayRemaining > 0.0f) continue;

		Pending.SpawnTimer -= DeltaTime;
		if (Pending.SpawnTimer > 0.0f) continue;

		// Spawn one enemy from this group
		ATDIMonsterSpawner* Spawner = FindSpawnerForTerritory(Pending.Group.SpawnTerritory);
		if (Spawner)
		{
			ATDIEnemyBase* Enemy = Spawner->SpawnEnemy(Pending.Group.EnemyClass);
			if (Enemy)
			{
				EnemiesAlive++;

				// Scale stats in endless mode
				if (bEndlessMode && EndlessMultiplier > 1.0f && Enemy->GetEnemyData())
				{
					// Stats are on the data asset; scale via override — controllers handle this
				}

				// Set AI objective
				if (ATDIEnemyAIController* AI = Cast<ATDIEnemyAIController>(Enemy->GetController()))
				{
					AI->SetObjective(Pending.Group.ForceObjective);
				}
			}
		}

		Pending.SpawnedCount++;
		Pending.SpawnTimer = Pending.Group.SpawnInterval;

		if (Pending.SpawnedCount >= Pending.Group.Count)
		{
			PendingGroups.RemoveAt(i);
		}
	}
}

void ATDIWaveManager::StartNextWave()
{
	CurrentWave++;

	const UTDIWaveData* WaveData = nullptr;

	if (WaveDataAssets.IsValidIndex(CurrentWave - 1))
	{
		WaveData = WaveDataAssets[CurrentWave - 1];
	}
	else if (bEndlessMode)
	{
		// Reuse the last wave definition with scaling
		WaveData = WaveDataAssets.IsEmpty() ? nullptr : WaveDataAssets.Last();
		EndlessMultiplier *= EndlessScalingMultiplier;
	}

	if (!WaveData) return;

	bWaveActive = true;
	EnemiesAlive = 0;
	PendingGroups.Empty();

	SpawnWave(WaveData);
	OnWaveStarted.Broadcast(CurrentWave);
}

void ATDIWaveManager::NotifyEnemyKilled()
{
	EnemiesAlive = FMath::Max(EnemiesAlive - 1, 0);

	if (bWaveActive && EnemiesAlive == 0 && PendingGroups.IsEmpty())
	{
		OnWaveComplete();
	}
}

void ATDIWaveManager::NotifyEnemyReachedCastle(ATDIEnemyBase* Enemy)
{
	NotifyEnemyKilled();   // Also counts as "removed from wave"
}

void ATDIWaveManager::SpawnWave(const UTDIWaveData* WaveData)
{
	if (!WaveData) return;

	for (const FTDIEnemyGroup& Group : WaveData->EnemyGroups)
	{
		if (!Group.EnemyClass) continue;

		FPendingGroup Pending;
		Pending.Group = Group;
		Pending.DelayRemaining = Group.WaveDelay;
		Pending.SpawnTimer = 0.0f;
		Pending.SpawnedCount = 0;

		// Pre-count total enemies
		EnemiesAlive += Group.Count;

		PendingGroups.Add(Pending);
	}
}

ATDIMonsterSpawner* ATDIWaveManager::FindSpawnerForTerritory(ETerritoryType TerritoryType) const
{
	TArray<AActor*> Spawners;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATDIMonsterSpawner::StaticClass(), Spawners);

	for (AActor* A : Spawners)
	{
		ATDIMonsterSpawner* Spawner = Cast<ATDIMonsterSpawner>(A);
		if (Spawner && Spawner->GetTerritoryType() == TerritoryType)
		{
			return Spawner;
		}
	}
	// Fallback: return any available spawner
	for (AActor* A : Spawners)
	{
		if (ATDIMonsterSpawner* Spawner = Cast<ATDIMonsterSpawner>(A)) return Spawner;
	}
	return nullptr;
}

void ATDIWaveManager::OnWaveComplete()
{
	bWaveActive = false;

	// Award completion bonus
	if (WaveDataAssets.IsValidIndex(CurrentWave - 1))
	{
		const UTDIWaveData* WaveData = WaveDataAssets[CurrentWave - 1];
		if (WaveData && WaveData->CompletionGoldBonus > 0)
		{
			if (UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>())
			{
				Resources->AddResource(TDITag_Resource_Gold,
					static_cast<float>(WaveData->CompletionGoldBonus));
			}
		}
	}

	OnWaveCompleted.Broadcast(CurrentWave);

	const bool bLastWave = !bEndlessMode && (CurrentWave >= WaveDataAssets.Num());
	if (bLastWave)
	{
		OnAllWavesCompleted.Broadcast();
	}
	else
	{
		// Reset preparation timer for next wave
		PreparationTimer = PreparationTime;
	}
}
