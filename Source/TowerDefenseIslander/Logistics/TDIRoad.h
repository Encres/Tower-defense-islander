#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TDIDataTypes.h"
#include "TDIRoad.generated.h"

class USplineComponent;
class USplineMeshComponent;
class ATDITerritoryBase;
class ATDICastle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoadStateChanged, ERoadState, NewState);

// A spline-based road segment connecting two points on the map.
// Roads form a directed graph; the logistics BFS uses them to determine
// which territories are reachable from the castle.
UCLASS()
class TOWERDEFENSEISLANDER_API ATDIRoad : public AActor
{
	GENERATED_BODY()

public:
	ATDIRoad();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// ---- Connectivity ----
	// Territory this road connects to (nullptr if it's a castle-side stub)
	UFUNCTION(BlueprintPure, Category = "Road")
	ATDITerritoryBase* GetConnectedTerritory() const { return ConnectedTerritory; }

	// True when one end of this road plugs into the castle's road node
	UFUNCTION(BlueprintPure, Category = "Road")
	bool IsConnectedToCastle() const { return bConnectedToCastle; }

	// True when this road shares a start/end actor with OtherRoad
	bool SharesEndpointWith(const ATDIRoad* OtherRoad) const;

	// ---- State ----
	UFUNCTION(BlueprintPure, Category = "Road")
	ERoadState GetRoadState() const { return RoadState; }

	UFUNCTION(BlueprintPure, Category = "Road")
	ERoadTier GetRoadTier() const { return RoadTier; }

	UFUNCTION(BlueprintPure, Category = "Road")
	float GetHealthPercent() const;

	// ---- Upgrade ----
	UFUNCTION(BlueprintCallable, Category = "Road")
	bool TryUpgradeTier();

	// ---- Repair ----
	UFUNCTION(BlueprintCallable, Category = "Road")
	void Repair(float Amount);

	// ---- Spline access (used by carts) ----
	UFUNCTION(BlueprintPure, Category = "Road")
	USplineComponent* GetSpline() const { return RoadSpline; }

	// ---- Save ----
	FTDIRoadSaveData GetSaveData() const;
	void LoadFromSaveData(const FTDIRoadSaveData& Data);

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Road|Events")
	FOnRoadStateChanged OnRoadStateChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> RoadSpline;

	// Territory at the "far" end of this road
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Road")
	TObjectPtr<ATDITerritoryBase> ConnectedTerritory;

	// Castle at the "near" end (or next road actor that connects to castle)
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Road")
	TObjectPtr<ATDICastle> NearCastle;

	// Road at the "near" end if this road chains to another rather than directly to castle
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Road")
	TObjectPtr<ATDIRoad> NearRoad;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Road")
	ERoadTier RoadTier = ERoadTier::Dirt;

	// Max HP per tier
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Road")
	TArray<float> MaxHealthPerTier = { 100.0f, 200.0f, 400.0f, 800.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Road")
	TArray<FTDIResourceAmount> UpgradeCostPerTier;

	// Speed multiplier granted to carts travelling on this road tier
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Road")
	TArray<float> CartSpeedMultiplierPerTier = { 1.0f, 1.2f, 1.5f, 2.0f };

private:
	ERoadState RoadState = ERoadState::Intact;
	float CurrentHealth = 100.0f;
	bool bConnectedToCastle = false;

	void UpdateRoadState();
};
