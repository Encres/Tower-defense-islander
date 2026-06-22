#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/TDIDataTypes.h"
#include "TDIResourceSubsystem.generated.h"

class UTDIResourceData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, FGameplayTag, ResourceType, float, NewAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStorageCapacityChanged, float, NewCapacity);

// Central ledger for all player-owned resources.
// All systems read/write resources exclusively through this subsystem.
UCLASS()
class TOWERDEFENSEISLANDER_API UTDIResourceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ---- Read ----
	UFUNCTION(BlueprintPure, Category = "Resources")
	float GetResource(FGameplayTag ResourceType) const;

	UFUNCTION(BlueprintPure, Category = "Resources")
	bool HasResources(const TArray<FTDIResourceAmount>& Costs) const;

	UFUNCTION(BlueprintPure, Category = "Resources")
	float GetStorageCapacity(FGameplayTag ResourceType) const;

	UFUNCTION(BlueprintPure, Category = "Resources")
	TMap<FGameplayTag, float> GetAllResources() const { return ResourceLedger; }

	// ---- Write ----
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void AddResource(FGameplayTag ResourceType, float Amount);

	// Returns true and deducts if affordable; false and no change if not
	UFUNCTION(BlueprintCallable, Category = "Resources")
	bool SpendResources(const TArray<FTDIResourceAmount>& Costs);

	// Direct spend without affordability check (for refunds, penalties)
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void DeductResource(FGameplayTag ResourceType, float Amount);

	// ---- Capacity ----
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void SetStorageCapacity(FGameplayTag ResourceType, float NewCapacity);

	UFUNCTION(BlueprintCallable, Category = "Resources")
	void SetGlobalCapacityMultiplier(float Multiplier);

	// ---- Debug ----
	UFUNCTION(BlueprintCallable, Category = "Resources|Debug")
	void AddAllResources(float Amount);

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Resources|Events")
	FOnResourceChanged OnResourceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Resources|Events")
	FOnStorageCapacityChanged OnStorageCapacityChanged;

	// ---- Serialization (used by save system) ----
	TMap<FString, float> SerializeToStringMap() const;
	void DeserializeFromStringMap(const TMap<FString, float>& Data);

private:
	// Tag → current amount
	TMap<FGameplayTag, float> ResourceLedger;

	// Tag → max storage capacity
	TMap<FGameplayTag, float> StorageCapacities;

	float GlobalCapacityMultiplier = 1.0f;

	float GetEffectiveCapacity(FGameplayTag ResourceType) const;
};
