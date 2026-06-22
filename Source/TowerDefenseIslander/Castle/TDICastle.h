#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TDIDataTypes.h"
#include "TDICastle.generated.h"

class UPaperSpriteComponent;
class UBoxComponent;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCastleHealthChanged, float, NewHealthPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastleDestroyed);

UCLASS()
class TOWERDEFENSEISLANDER_API ATDICastle : public AActor
{
	GENERATED_BODY()

public:
	ATDICastle();

	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// Called by the logistics subsystem to identify this as the delivery target
	FVector GetResourceDropOffLocation() const;

	// ---- Health ----
	UFUNCTION(BlueprintPure, Category = "Castle")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Castle")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Castle")
	float GetMaxHealth() const { return MaxHealth; }

	// ---- Upgrade ----
	UFUNCTION(BlueprintPure, Category = "Castle")
	int32 GetCastleLevel() const { return CastleLevel; }

	UFUNCTION(BlueprintCallable, Category = "Castle")
	bool TryUpgradeCastle();

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Castle|Events")
	FOnCastleHealthChanged OnCastleHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Castle|Events")
	FOnCastleDestroyed OnCastleDestroyed;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> ResourceDropOffPoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Castle|Stats", meta = (ClampMin = "100"))
	float MaxHealth = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Castle|Upgrade")
	int32 MaxCastleLevel = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Castle|Upgrade")
	TArray<FTDIResourceCost> UpgradeCosts;  // one entry per upgrade tier

private:
	float CurrentHealth = 0.0f;
	int32 CastleLevel = 1;

	void ApplyLevelUpgrade();
};
