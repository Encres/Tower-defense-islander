#include "TDIMonsterSpawner.h"
#include "Enemies/TDIEnemyBase.h"
#include "TimerManager.h"

ATDIMonsterSpawner::ATDIMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATDIMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(PatrolTimer, this,
		&ATDIMonsterSpawner::SpawnPatrolEnemy, PatrolSpawnInterval, true);
}

void ATDIMonsterSpawner::SetCapture(bool bCaptured)
{
	bIsCaptured = bCaptured;

	if (bCaptured)
	{
		GetWorldTimerManager().PauseTimer(PatrolTimer);
		// Despawn active patrol enemies
		for (TObjectPtr<ATDIEnemyBase> Enemy : ActivePatrolEnemies)
		{
			if (Enemy) Enemy->Destroy();
		}
		ActivePatrolEnemies.Empty();
	}
	else
	{
		GetWorldTimerManager().UnPauseTimer(PatrolTimer);
	}
}

ATDIEnemyBase* ATDIMonsterSpawner::SpawnEnemy(TSubclassOf<ATDIEnemyBase> EnemyClass)
{
	if (!EnemyClass) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ATDIEnemyBase* Enemy = GetWorld()->SpawnActor<ATDIEnemyBase>(
		EnemyClass, GetActorLocation(), GetActorRotation(), Params);
	return Enemy;
}

void ATDIMonsterSpawner::SpawnPatrolEnemy()
{
	if (bIsCaptured) return;
	if (PatrolEnemyClasses.IsEmpty()) return;

	// Remove dead patrol enemies from list
	ActivePatrolEnemies.RemoveAll([](const TObjectPtr<ATDIEnemyBase>& E) { return !E.IsValid(); });

	if (ActivePatrolEnemies.Num() >= MaxPatrolEnemies) return;

	const int32 Idx = FMath::RandRange(0, PatrolEnemyClasses.Num() - 1);
	ATDIEnemyBase* Enemy = SpawnEnemy(PatrolEnemyClasses[Idx]);
	if (Enemy)
	{
		ActivePatrolEnemies.Add(Enemy);
	}
}
