#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Data/TDIDataTypes.h"
#include "TDIHUDBase.generated.h"

class ATDITowerBase;
class ATDITerritoryBase;
class ATDIRoad;

// C++ base for all HUD widgets.
// Exposes BlueprintImplementableEvents so designers implement the visual
// updates entirely in Blueprint/UMG without any C++ coupling.
UCLASS()
class TOWERDEFENSEISLANDER_API UTDIHUDBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ---- Binding points (called from C++ systems) ----

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Phase")
	void OnPhaseChanged(EGamePhase NewPhase);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Resources")
	void OnResourceChanged(FGameplayTag ResourceType, float NewAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Waves")
	void OnWaveStarted(int32 WaveNumber, int32 TotalWaves);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Waves")
	void OnPreparationTimerUpdated(float Remaining, float Total);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Selection")
	void OnTowerSelected(ATDITowerBase* Tower);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Selection")
	void OnTerritorySelected(ATDITerritoryBase* Territory);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Selection")
	void OnRoadSelected(ATDIRoad* Road);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Selection")
	void OnSelectionCleared();

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Research")
	void OnResearchUnlocked(FGameplayTag ResearchTag);

	// ---- Queries (callable from Blueprint HUD logic) ----

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetResourceAmount(FGameplayTag ResourceType) const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	int32 GetCurrentWave() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	EGamePhase GetCurrentPhase() const;

protected:
	// Cached references wired during NativeConstruct
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	TObjectPtr<class ATDIGameMode> GameMode;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	TObjectPtr<class UTDIResourceSubsystem> ResourceSubsystem;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	TObjectPtr<class ATDIWaveManager> WaveManager;

private:
	UFUNCTION()
	void HandleResourceChanged(FGameplayTag ResourceType, float NewAmount);

	UFUNCTION()
	void HandleWaveStarted(int32 WaveNumber);

	UFUNCTION()
	void HandlePreparationTimer(float Remaining, float Total);

	UFUNCTION()
	void HandleResearchUnlocked(FGameplayTag ResearchTag);
};
