#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TDIDataTypes.h"
#include "TDIResearchData.generated.h"

// A single node in the research / tech tree.
UCLASS(BlueprintType)
class TOWERDEFENSEISLANDER_API UTDIResearchData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Unique tag identifying this research unlock
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Research")
	FGameplayTag ResearchTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Research")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Research")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Research|Visual")
	TObjectPtr<UTexture2D> Icon;

	// Resources consumed to unlock this research
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Research|Cost")
	TArray<FTDIResourceCost> Cost;

	// Research nodes that must be unlocked before this one
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Research|Prerequisites")
	TArray<FGameplayTag> Prerequisites;

	// Research that becomes available after this one is unlocked
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Research|Unlocks")
	TArray<FGameplayTag> Unlocks;

	// Time in seconds to complete research (if using time-based research)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Research|Economy", meta = (ClampMin = "0"))
	float ResearchTime = 30.0f;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ResearchData", GetFName());
	}
};
