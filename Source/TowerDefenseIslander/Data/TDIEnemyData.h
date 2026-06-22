#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TDIDataTypes.h"
#include "TDIEnemyData.generated.h"

// Data asset describing an enemy type.
// Create one per enemy (e.g., DA_Enemy_Goblin) in Content/Data/Enemies/.
UCLASS(BlueprintType)
class TOWERDEFENSEISLANDER_API UTDIEnemyData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	ETerritoryType HomeTerritory = ETerritoryType::GoblinCamp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	EEnemyMoveType MoveType = EEnemyMoveType::Ground;

	// --- Combat Stats ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats", meta = (ClampMin = "1"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats", meta = (ClampMin = "1"))
	float MoveSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats", meta = (ClampMin = "0"))
	float AttackDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats", meta = (ClampMin = "0"))
	float AttackRate = 1.0f;     // attacks per second

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats", meta = (ClampMin = "0"))
	float AttackRange = 100.0f;

	// Armor reduces incoming physical damage by this fraction (0–1)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats", meta = (ClampMin = "0", ClampMax = "0.95"))
	float ArmorReduction = 0.0f;

	// --- Rewards ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Reward", meta = (ClampMin = "0"))
	int32 GoldReward = 10;

	// --- AI Behaviour ---
	// What this enemy prefers to target
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|AI")
	EEnemyObjective PrimaryObjective = EEnemyObjective::AttackCastle;

	// Tags granted to this enemy (e.g. "Enemy.Undead", "Enemy.Boss")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Tags")
	FGameplayTagContainer EnemyTags;

	// Tags this enemy is immune to (e.g. "Effect.Poison" for undead)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Resistance")
	FGameplayTagContainer ImmuneTags;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("EnemyData", GetFName());
	}
};
