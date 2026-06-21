#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDITowerBase.generated.h"

class UPaperSpriteComponent;
class USphereComponent;
class ATDIEnemyBase;

USTRUCT(BlueprintType)
struct FTowerStats
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Range = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FireRate = 1.0f;     // shots per second

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Damage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Cost = 50;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 UpgradeCost = 75;
};

UCLASS(Abstract)
class TOWERDEFENSEISLANDER_API ATDITowerBase : public AActor
{
	GENERATED_BODY()

public:
	ATDITowerBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Called after the player confirms placement
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Tower")
	void OnPlaced();

	// Toggles ghost (preview) appearance — translucent, no collision
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void SetGhostMode(bool bGhost);

	// Returns true when this grid cell is a valid placement spot
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Tower")
	bool CanPlaceHere() const;

	UFUNCTION(BlueprintPure, Category = "Tower")
	const FTowerStats& GetStats() const { return Stats; }

	UFUNCTION(BlueprintCallable, Category = "Tower")
	bool TryUpgrade();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPaperSpriteComponent* SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* RangeCollider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	FTowerStats Stats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	int32 MaxUpgradeLevel = 3;

	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	int32 CurrentUpgradeLevel = 0;

	// Target selection — returns the nearest enemy within range, or nullptr
	UFUNCTION(BlueprintCallable, Category = "Tower")
	ATDIEnemyBase* FindTarget() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Tower")
	void FireAtTarget(ATDIEnemyBase* Target);

	virtual void FireAtTarget_Implementation(ATDIEnemyBase* Target);

private:
	bool bIsGhost = false;
	float FireCooldown = 0.0f;

	UFUNCTION()
	void OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	TArray<TWeakObjectPtr<ATDIEnemyBase>> EnemiesInRange;
};
