#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "TDIResourceData.generated.h"

// Designer-facing data asset describing a single resource type.
// Create one per resource (e.g., DA_Resource_Wood) in Content/Data/Resources/.
UCLASS(BlueprintType)
class TOWERDEFENSEISLANDER_API UTDIResourceData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Unique gameplay tag identifying this resource (e.g. "Resource.Wood")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	FGameplayTag ResourceTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	FText Description;

	// Icon shown in HUD resource bar
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Visual")
	TObjectPtr<UTexture2D> Icon;

	// Tint color for this resource in the UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Visual")
	FLinearColor DisplayColor = FLinearColor::White;

	// Maximum amount the castle can store (before storage upgrades)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Economy", meta = (ClampMin = "1"))
	float BaseStorageCapacity = 500.0f;

	// Which territory type produces this resource
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Economy")
	ETerritoryType SourceTerritory;

	// Base units produced per production tick at the resource node
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Economy", meta = (ClampMin = "0.1"))
	float ProductionPerTick = 10.0f;

	// Gold sell value per unit (for future selling mechanic)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Economy", meta = (ClampMin = "0"))
	float GoldValuePerUnit = 1.0f;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ResourceData", GetFName());
	}
};
