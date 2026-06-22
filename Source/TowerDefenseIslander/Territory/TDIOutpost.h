#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TDIDataTypes.h"
#include "TDIOutpost.generated.h"

class UPaperSpriteComponent;
class UBoxComponent;
class ATDITerritoryBase;
class ATDIResourceNode;

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIOutpost : public AActor
{
	GENERATED_BODY()

public:
	ATDIOutpost();

	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	void Initialize(ATDITerritoryBase* OwnerTerritory);

	// Enable/disable resource production (e.g. if road is broken)
	UFUNCTION(BlueprintCallable, Category = "Outpost")
	void SetActive(bool bActive);

	UFUNCTION(BlueprintPure, Category = "Outpost")
	bool IsActive() const { return bIsActive; }

	// ---- Resource Storage ----
	// Called by resource nodes to deposit produced resources locally
	void DepositResource(FGameplayTag ResourceType, float Amount);

	// Called by worker carts to pick up available resources
	TArray<FTDIResourceAmount> PickupResources(float CartCapacity);

	UFUNCTION(BlueprintPure, Category = "Outpost")
	float GetStoredAmount(FGameplayTag ResourceType) const;

	UFUNCTION(BlueprintPure, Category = "Outpost")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Outpost")
	ATDITerritoryBase* GetOwnerTerritory() const { return OwnerTerritory; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Outpost", meta = (ClampMin = "50"))
	float MaxHealth = 300.0f;

	// Maximum resource units stored locally before carts are mandatory
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Outpost", meta = (ClampMin = "10"))
	float LocalStorageCapacity = 200.0f;

private:
	float CurrentHealth = 0.0f;
	bool bIsActive = false;

	UPROPERTY()
	TObjectPtr<ATDITerritoryBase> OwnerTerritory;

	// Local buffer: resources waiting for a cart
	TMap<FGameplayTag, float> LocalStorage;
};
