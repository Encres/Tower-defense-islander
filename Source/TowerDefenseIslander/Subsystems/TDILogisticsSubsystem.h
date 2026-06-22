#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/TDIDataTypes.h"
#include "TDILogisticsSubsystem.generated.h"

class ATDIRoad;
class ATDICastle;
class ATDIWorkerCart;
class ATDITerritoryBase;
class ATDIOutpost;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoadNetworkChanged, bool, bAnyRoadBroken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRouteConnectivityChanged, ATDITerritoryBase*, Territory, bool, bIsNowConnected);

// Manages the entire road network.
// Tracks which roads exist, validates connectivity via BFS from the castle,
// and assigns / manages worker carts for each territory route.
UCLASS()
class TOWERDEFENSEISLANDER_API UTDILogisticsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ---- Registration ----
	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void RegisterRoad(ATDIRoad* Road);

	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void UnregisterRoad(ATDIRoad* Road);

	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void RegisterCastle(ATDICastle* Castle);

	// ---- Network Validation ----
	// Re-runs BFS from castle; updates all territory connectivity flags.
	// Call this whenever a road is built, destroyed, or repaired.
	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void ValidateNetwork();

	UFUNCTION(BlueprintPure, Category = "Logistics")
	bool IsTerritoryConnected(ATDITerritoryBase* Territory) const;

	UFUNCTION(BlueprintPure, Category = "Logistics")
	TArray<ATDIRoad*> GetAllRoads() const { return AllRoads; }

	UFUNCTION(BlueprintPure, Category = "Logistics")
	TArray<ATDIRoad*> GetConnectedRoads() const;

	// ---- Cart Management ----
	// Spawn the correct number of carts for a territory based on research/upgrades
	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void SpawnCartsForTerritory(ATDITerritoryBase* Territory);

	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void DespawnCartsForTerritory(ATDITerritoryBase* Territory);

	// Called when a cart is destroyed; respawns after a delay
	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void OnCartDestroyed(ATDIWorkerCart* Cart);

	UFUNCTION(BlueprintPure, Category = "Logistics")
	int32 GetBaseCartsPerTerritory() const { return BaseCartsPerTerritory; }

	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void SetBaseCartsPerTerritory(int32 NewCount);

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Logistics|Events")
	FOnRoadNetworkChanged OnRoadNetworkChanged;

	UPROPERTY(BlueprintAssignable, Category = "Logistics|Events")
	FOnRouteConnectivityChanged OnRouteConnectivityChanged;

	// ---- Save helpers ----
	TArray<FTDIRoadSaveData> SerializeRoads() const;

private:
	UPROPERTY()
	TArray<TObjectPtr<ATDIRoad>> AllRoads;

	UPROPERTY()
	TObjectPtr<ATDICastle> CastleActor;

	// Territory → is currently reachable from castle
	TMap<ATDITerritoryBase*, bool> ConnectivityMap;

	// Territory → active carts
	TMap<ATDITerritoryBase*, TArray<TObjectPtr<ATDIWorkerCart>>> CartsByTerritory;

	int32 BaseCartsPerTerritory = 2;
	float CartRespawnDelay = 10.0f;

	void BFSFromCastle(TSet<ATDIRoad*>& OutReachable) const;
};
