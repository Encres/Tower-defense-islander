#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/TDIDataTypes.h"
#include "TDIGameMode.generated.h"

class ATDITopDownPawn;
class ATDIPlayerController;
class ATDIWaveManager;
class ATDIResearchManager;
class ATDICastle;
class UTDIHUDBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATDIGameMode();

	virtual void BeginPlay() override;

	// ---- Phase ----
	UFUNCTION(BlueprintPure, Category = "GameMode")
	EGamePhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void SetPhase(EGamePhase NewPhase);

	// ---- Convenience accessors ----
	UFUNCTION(BlueprintPure, Category = "GameMode")
	ATDIWaveManager* GetWaveManager() const { return CachedWaveManager; }

	UFUNCTION(BlueprintPure, Category = "GameMode")
	ATDIResearchManager* GetResearchManager() const { return CachedResearchManager; }

	UFUNCTION(BlueprintPure, Category = "GameMode")
	ATDICastle* GetCastle() const { return CachedCastle; }

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
	FOnGamePhaseChanged OnGamePhaseChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode")
	TSubclassOf<UTDIHUDBase> HUDWidgetClass;

private:
	EGamePhase CurrentPhase = EGamePhase::Preparation;

	UPROPERTY()
	TObjectPtr<ATDIWaveManager> CachedWaveManager;

	UPROPERTY()
	TObjectPtr<ATDIResearchManager> CachedResearchManager;

	UPROPERTY()
	TObjectPtr<ATDICastle> CachedCastle;

	UPROPERTY()
	TObjectPtr<UTDIHUDBase> HUDWidget;

	UFUNCTION()
	void OnCastleDestroyed();

	UFUNCTION()
	void OnAllWavesCompleted();

	UFUNCTION()
	void OnWaveStarted(int32 WaveNumber);

	UFUNCTION()
	void OnWaveCompleted(int32 WaveNumber);

	void CacheActors();
	void CreateHUD();
};
