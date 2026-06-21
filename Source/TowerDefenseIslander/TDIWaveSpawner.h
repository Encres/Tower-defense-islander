#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDIWaveSpawner.generated.h"

class USplineComponent;
class ATDIEnemyBase;

USTRUCT(BlueprintType)
struct FEnemySpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ATDIEnemyBase> EnemyClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Count = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpawnInterval = 1.5f;   // seconds between individual enemy spawns
};

USTRUCT(BlueprintType)
struct FWaveDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FEnemySpawnEntry> Groups;
};

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	ATDIWaveSpawner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Called by GameMode when a wave begins
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartWave(int32 WaveIndex);

protected:
	// Spline that defines the enemy path across the island
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawner")
	USplineComponent* EnemyPath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawner")
	TArray<FWaveDefinition> WaveDefinitions;

private:
	int32 CurrentGroup = 0;
	int32 SpawnedInGroup = 0;
	float SpawnTimer = 0.0f;
	bool bSpawning = false;
	int32 ActiveWaveIndex = 0;

	void SpawnNext();
};
