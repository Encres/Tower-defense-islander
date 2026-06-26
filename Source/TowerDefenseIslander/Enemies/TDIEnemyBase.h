#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Data/TDIDataTypes.h"
#include "TDIEnemyBase.generated.h"

class UPaperSpriteComponent;
class UTDIEnemyData;
class ATDIEnemyAIController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDied, ATDIEnemyBase*, Enemy);

UCLASS(Abstract)
class TOWERDEFENSEISLANDER_API ATDIEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	ATDIEnemyBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// ---- Status Effects ----
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyStatusEffect(const FTDIStatusEffect& Effect);

	// ---- State ----
	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetPathProgress() const { return PathProgress; }

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetPathProgress(float Progress) { PathProgress = Progress; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetCurrentSpeedMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	const FGameplayTagContainer& GetEnemyTags() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	const UTDIEnemyData* GetEnemyData() const { return EnemyData; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	EEnemyObjective GetPrimaryObjective() const;

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FOnEnemyDied OnEnemyDied;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Data")
	TObjectPtr<UTDIEnemyData> EnemyData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy")
	void OnDeath();
	virtual void OnDeath_Implementation();

private:
	float CurrentHealth = 0.0f;
	bool bIsDead = false;
	float PathProgress = 0.0f;     // 0–1: set by AI controller

	// Active status effects
	TArray<FTDIStatusEffect> ActiveEffects;

	void TickStatusEffects(float DeltaTime);
	float ComputeSpeedMultiplier() const;
};
