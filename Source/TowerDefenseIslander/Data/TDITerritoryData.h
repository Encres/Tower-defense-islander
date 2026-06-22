#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TDIDataTypes.h"
#include "TDITerritoryData.generated.h"

class ATDIEnemyBase;

// Data asset describing a territory type and its properties.
UCLASS(BlueprintType)
class TOWERDEFENSEISLANDER_API UTDITerritoryData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory")
	ETerritoryType TerritoryType = ETerritoryType::GoblinCamp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory")
	FText CaptureDescription;

	// Resources this territory produces when captured
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory|Resources")
	TArray<FTDIResourceAmount> ResourcesProduced;

	// How many seconds it takes to fully capture this territory (uncontested)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory|Capture", meta = (ClampMin = "10"))
	float CaptureTime = 120.0f;

	// Gold cost to initiate a capture attempt
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory|Capture", meta = (ClampMin = "0"))
	int32 CaptureCostGold = 100;

	// Research unlocked when this territory is first captured
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory|Research")
	FGameplayTag ResearchUnlockedOnCapture;

	// Enemy types that defend this territory
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory|Enemies")
	TArray<TSubclassOf<ATDIEnemyBase>> DefendingEnemyClasses;

	// How many enemies respawn each wave at this territory
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory|Enemies", meta = (ClampMin = "1"))
	int32 BaseDefenderCount = 4;

	// The "boss" enemy that appears on every 5th wave
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory|Enemies")
	TSubclassOf<ATDIEnemyBase> TerritoryBossClass;

	// Color tint for map overlay
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory|Visual")
	FLinearColor MapColor = FLinearColor::Red;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("TerritoryData", GetFName());
	}
};
