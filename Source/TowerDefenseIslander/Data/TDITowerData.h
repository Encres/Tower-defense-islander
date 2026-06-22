#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TDIDataTypes.h"
#include "TDITowerData.generated.h"

class ATDIProjectileBase;

// All stats and upgrade information for one tower type across all tiers.
// Create one per tower type (e.g., DA_Tower_Arrow) in Content/Data/Towers/.
UCLASS(BlueprintType)
class TOWERDEFENSEISLANDER_API UTDITowerData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	ETowerType TowerType = ETowerType::Arrow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Visual")
	TObjectPtr<UTexture2D> Icon;

	// Projectile class this tower fires (nullptr = instant hit)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Gameplay")
	TSubclassOf<ATDIProjectileBase> ProjectileClass;

	// Initial placement cost in Gold
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Economy", meta = (ClampMin = "0"))
	int32 PlacementCostGold = 50;

	// Stats per tier — must have exactly 4 entries (Tier1–Tier4)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Stats")
	TArray<FTDITowerTierStats> TierStats;

	// Research tag required before this tower can be placed at all
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Research")
	FGameplayTag RequiredResearchToPlace;

	// Enemies this tower deals bonus damage against (tag match)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Gameplay")
	FGameplayTagContainer BonusDamageAgainst;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Gameplay", meta = (ClampMin = "1.0"))
	float BonusDamageMultiplier = 2.0f;

	// Targeting priority: true = attacks enemy furthest along path
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Gameplay")
	bool bPrioritizeFurthest = true;

	const FTDITowerTierStats* GetTierStats(ETowerTier Tier) const
	{
		int32 Idx = static_cast<int32>(Tier);
		return TierStats.IsValidIndex(Idx) ? &TierStats[Idx] : nullptr;
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("TowerData", GetFName());
	}
};
