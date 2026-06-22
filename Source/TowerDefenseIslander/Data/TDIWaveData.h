#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TDIDataTypes.h"
#include "TDIWaveData.generated.h"

class ATDIEnemyBase;

// One group of enemies that spawns together during a wave
USTRUCT(BlueprintType)
struct TOWERDEFENSEISLANDER_API FTDIEnemyGroup
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ATDIEnemyBase> EnemyClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 Count = 5;

	// Seconds between individual enemy spawns within this group
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.1"))
	float SpawnInterval = 1.5f;

	// Seconds after wave start before this group begins spawning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0"))
	float WaveDelay = 0.0f;

	// Preferred territory spawner to use (Wild = use any)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ETerritoryType SpawnTerritory = ETerritoryType::GoblinCamp;

	// If true, overrides PrimaryObjective from EnemyData to target roads
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EEnemyObjective ForceObjective = EEnemyObjective::AttackCastle;
};

// Full definition for one wave
UCLASS(BlueprintType)
class TOWERDEFENSEISLANDER_API UTDIWaveData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	int32 WaveNumber = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	FText WaveTitle;

	// All enemy groups in this wave (can spawn simultaneously from different territories)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	TArray<FTDIEnemyGroup> EnemyGroups;

	// Gold bonus awarded on wave completion
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave|Reward", meta = (ClampMin = "0"))
	int32 CompletionGoldBonus = 50;

	// Whether this is a boss wave (spawns territory bosses)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	bool bIsBossWave = false;

	// Global HP/damage multiplier applied to all enemies in this wave
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave|Scaling", meta = (ClampMin = "0.1"))
	float DifficultyMultiplier = 1.0f;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("WaveData", GetFName());
	}
};
