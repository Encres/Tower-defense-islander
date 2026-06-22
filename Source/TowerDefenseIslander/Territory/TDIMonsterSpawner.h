#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TDIDataTypes.h"
#include "TDIMonsterSpawner.generated.h"

class ATDIEnemyBase;

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIMonsterSpawner : public AActor
{
	GENERATED_BODY()

public:
	ATDIMonsterSpawner();

	virtual void BeginPlay() override;

	// Called when territory is captured: suppress enemy spawning
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SetCapture(bool bCaptured);

	// Wave manager calls this to spawn a specific enemy class
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	ATDIEnemyBase* SpawnEnemy(TSubclassOf<ATDIEnemyBase> EnemyClass);

	UFUNCTION(BlueprintPure, Category = "Spawner")
	bool IsCaptured() const { return bIsCaptured; }

	UFUNCTION(BlueprintPure, Category = "Spawner")
	ETerritoryType GetTerritoryType() const { return AssociatedTerritoryType; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawner")
	ETerritoryType AssociatedTerritoryType = ETerritoryType::GoblinCamp;

	// Default patrol enemies that spawn outside of wave events
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawner")
	TArray<TSubclassOf<ATDIEnemyBase>> PatrolEnemyClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "30"))
	float PatrolSpawnInterval = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "1"))
	int32 MaxPatrolEnemies = 4;

private:
	bool bIsCaptured = false;
	FTimerHandle PatrolTimer;

	UPROPERTY()
	TArray<TObjectPtr<ATDIEnemyBase>> ActivePatrolEnemies;

	UFUNCTION()
	void SpawnPatrolEnemy();
};
