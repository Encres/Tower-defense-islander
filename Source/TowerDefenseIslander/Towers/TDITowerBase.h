#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TDIDataTypes.h"
#include "TDITowerBase.generated.h"

class UPaperSpriteComponent;
class USphereComponent;
class ATDIEnemyBase;
class ATDIProjectileBase;
class UTDITowerData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTowerUpgraded, int32, NewTier);

UCLASS(Abstract)
class TOWERDEFENSEISLANDER_API ATDITowerBase : public AActor
{
	GENERATED_BODY()

public:
	ATDITowerBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// ---- Placement / Ghost ----
	UFUNCTION(BlueprintCallable, Category = "Tower|Placement")
	void SetGhostMode(bool bGhost);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Tower|Placement")
	void OnPlaced();
	virtual void OnPlaced_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Tower|Placement")
	bool CanPlaceHere() const;
	virtual bool CanPlaceHere_Implementation() const;

	// ---- Upgrade ----
	UFUNCTION(BlueprintCallable, Category = "Tower|Upgrade")
	bool TryUpgrade();

	UFUNCTION(BlueprintPure, Category = "Tower|Upgrade")
	ETowerTier GetCurrentTier() const { return CurrentTier; }

	UFUNCTION(BlueprintPure, Category = "Tower|Upgrade")
	bool CanUpgrade() const;

	UFUNCTION(BlueprintPure, Category = "Tower|Upgrade")
	TArray<FTDIResourceCost> GetUpgradeCost() const;

	// ---- Stats ----
	UFUNCTION(BlueprintPure, Category = "Tower")
	const UTDITowerData* GetTowerData() const { return TowerData; }

	UFUNCTION(BlueprintPure, Category = "Tower")
	float GetHealthPercent() const;

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Tower|Events")
	FOnTowerUpgraded OnTowerUpgraded;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> RangeSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Data")
	TObjectPtr<UTDITowerData> TowerData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Stats", meta = (ClampMin = "50"))
	float MaxHealth = 200.0f;

	// Select which enemy to fire at next (override for custom targeting logic)
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Tower|Combat")
	ATDIEnemyBase* SelectTarget() const;
	virtual ATDIEnemyBase* SelectTarget_Implementation() const;

	// Perform one attack on the selected target (override for special effects)
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Tower|Combat")
	void Fire(ATDIEnemyBase* Target);
	virtual void Fire_Implementation(ATDIEnemyBase* Target);

	// Apply status effects — called by Fire; override to customize per tower type
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Tower|Combat")
	void ApplyStatusEffect(ATDIEnemyBase* Target, const FTDITowerTierStats& Stats);
	virtual void ApplyStatusEffect_Implementation(ATDIEnemyBase* Target, const FTDITowerTierStats& Stats);

	virtual void OnTierChanged(ETowerTier NewTier) {}

private:
	ETowerTier CurrentTier = ETowerTier::Tier1;
	float CurrentHealth = 0.0f;
	float FireCooldown = 0.0f;
	bool bIsGhost = false;

	UPROPERTY()
	TArray<TWeakObjectPtr<ATDIEnemyBase>> EnemiesInRange;

	UFUNCTION()
	void OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyTierStats();
};
