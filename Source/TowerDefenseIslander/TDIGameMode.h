#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TDIGameMode.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Preparation,
	Wave,
	GameOver,
	Victory
};

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATDIGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartWave();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void EndWave();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnBaseDestroyed();

	UFUNCTION(BlueprintPure, Category = "Game")
	EGamePhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "Game")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "Game")
	int32 GetTotalWaves() const { return TotalWaves; }

	UFUNCTION(BlueprintPure, Category = "Game")
	int32 GetPlayerGold() const { return PlayerGold; }

	UFUNCTION(BlueprintCallable, Category = "Game")
	bool SpendGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void AddGold(int32 Amount);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	int32 TotalWaves = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	float PreparationTime = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
	int32 StartingGold = 150;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
	int32 GoldPerWave = 50;

private:
	EGamePhase CurrentPhase = EGamePhase::Preparation;
	int32 CurrentWave = 0;
	int32 PlayerGold = 0;
	float PhaseTimer = 0.0f;
	int32 EnemiesRemainingInWave = 0;

	void ChangePhase(EGamePhase NewPhase);

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EGamePhase, NewPhase);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewAmount);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveNumber);

	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FOnPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FOnGoldChanged OnGoldChanged;

	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FOnWaveStarted OnWaveStarted;

	void NotifyEnemyKilled();
};
