#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Data/TDIDataTypes.h"
#include "TDIResearchManager.generated.h"

class UTDIResearchData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResearchUnlocked, FGameplayTag, ResearchTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResearchStarted, UTDIResearchData*, ResearchNode);

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIResearchManager : public AActor
{
	GENERATED_BODY()

public:
	ATDIResearchManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ---- Queries ----
	UFUNCTION(BlueprintPure, Category = "Research")
	bool IsUnlocked(FGameplayTag ResearchTag) const;

	UFUNCTION(BlueprintPure, Category = "Research")
	bool CanUnlock(UTDIResearchData* Research) const;

	UFUNCTION(BlueprintPure, Category = "Research")
	bool IsResearching() const { return CurrentResearch != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Research")
	UTDIResearchData* GetCurrentResearch() const { return CurrentResearch; }

	UFUNCTION(BlueprintPure, Category = "Research")
	float GetResearchProgress() const;

	UFUNCTION(BlueprintPure, Category = "Research")
	TArray<UTDIResearchData*> GetAvailableResearch() const;

	// ---- Actions ----
	// Begin researching a node (deducts cost, starts timer)
	UFUNCTION(BlueprintCallable, Category = "Research")
	bool StartResearch(UTDIResearchData* Research);

	// Cancel the current research (refunds cost)
	UFUNCTION(BlueprintCallable, Category = "Research")
	void CancelResearch();

	// Instantly unlock a tag (used by territory capture rewards)
	UFUNCTION(BlueprintCallable, Category = "Research")
	void UnlockByTag(FGameplayTag ResearchTag);

	// ---- Serialization ----
	TArray<FString> SerializeUnlocks() const;
	void DeserializeUnlocks(const TArray<FString>& TagStrings);

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Research|Events")
	FOnResearchUnlocked OnResearchUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Research|Events")
	FOnResearchStarted OnResearchStarted;

protected:
	// All research nodes available in this game mode (set in editor or via data)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Research")
	TArray<TObjectPtr<UTDIResearchData>> AllResearchNodes;

private:
	TSet<FGameplayTag> UnlockedResearch;

	UPROPERTY()
	TObjectPtr<UTDIResearchData> CurrentResearch;

	float ResearchTimer = 0.0f;
	TArray<FTDIResourceAmount> RefundCache;
};
