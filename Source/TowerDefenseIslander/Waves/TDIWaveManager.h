#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TDIDataTypes.h"
#include "TDIWaveManager.generated.h"

class UTDIWaveData;
class ATDIEnemyBase;
class ATDIMonsterSpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCompleted, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPreparationTimerUpdated, float, Remaining, float, Total);

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIWaveManager : public AActor
{
	GENERATED_BODY()

public:
	ATDIWaveManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Called by GameMode to trigger next wave
	UFUNCTION(BlueprintCallable, Category = "Waves")
	void StartNextWave();

	// Called by enemies when they die (used to detect wave completion)
	UFUNCTION(BlueprintCallable, Category = "Waves")
	void NotifyEnemyKilled();

	// Called by enemies that reach the castle
	UFUNCTION(BlueprintCallable, Category = "Waves")
	void NotifyEnemyReachedCastle(ATDIEnemyBase* Enemy);

	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetCurrentWaveNumber() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetTotalWaves() const { return WaveDataAssets.Num(); }

	UFUNCTION(BlueprintPure, Category = "Waves")
	float GetPreparationTimeRemaining() const { return PreparationTimer; }

	UFUNCTION(BlueprintPure, Category = "Waves")
	bool IsWaveActive() const { return bWaveActive; }

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Waves|Events")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Waves|Events")
	FOnWaveCompleted OnWaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Waves|Events")
	FOnAllWavesCompleted OnAllWavesCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Waves|Events")
	FOnPreparationTimerUpdated OnPreparationTimerUpdated;

protected:
	// Wave data assets ordered by wave number (Wave 1 = index 0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Waves")
	TArray<TObjectPtr<UTDIWaveData>> WaveDataAssets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Waves", meta = (ClampMin = "10"))
	float PreparationTime = 90.0f;

	// In endless mode, this multiplier is applied to enemy stats after all waves complete
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Waves|Endless", meta = (ClampMin = "1.0"))
	float EndlessScalingMultiplier = 1.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Waves")
	bool bEndlessMode = false;

private:
	int32 CurrentWave = 0;
	int32 EnemiesAlive = 0;
	bool bWaveActive = false;
	float PreparationTimer = 0.0f;
	float EndlessMultiplier = 1.0f;

	// Pending spawn groups: {WaveData group index, spawner, delay remaining}
	struct FPendingGroup
	{
		FTDIEnemyGroup Group;
		float DelayRemaining;
		int32 SpawnedCount;
		float SpawnTimer;
	};
	TArray<FPendingGroup> PendingGroups;

	void SpawnWave(const UTDIWaveData* WaveData);
	ATDIMonsterSpawner* FindSpawnerForTerritory(ETerritoryType TerritoryType) const;
	void OnWaveComplete();
};
