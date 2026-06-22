#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/TDIDataTypes.h"
#include "TDITerritorySubsystem.generated.h"

class ATDITerritoryBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerritoryStateChanged, ATDITerritoryBase*, Territory, ETerritoryState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTerritoryRegistered, ATDITerritoryBase*, Territory);

// Tracks every territory on the map and their capture states.
// Notifies logistics subsystem when a territory is captured so roads can be validated.
UCLASS()
class TOWERDEFENSEISLANDER_API UTDITerritorySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Called by ATDITerritoryBase on BeginPlay
	UFUNCTION(BlueprintCallable, Category = "Territories")
	void RegisterTerritory(ATDITerritoryBase* Territory);

	UFUNCTION(BlueprintCallable, Category = "Territories")
	void UnregisterTerritory(ATDITerritoryBase* Territory);

	// Attempt to initiate capture of a territory (deducts gold, starts progress)
	UFUNCTION(BlueprintCallable, Category = "Territories")
	bool BeginCaptureAttempt(ATDITerritoryBase* Territory);

	// Called by the territory actor when its capture completes
	UFUNCTION(BlueprintCallable, Category = "Territories")
	void NotifyTerritoryStatChanged(ATDITerritoryBase* Territory, ETerritoryState NewState);

	// ---- Queries ----
	UFUNCTION(BlueprintPure, Category = "Territories")
	TArray<ATDITerritoryBase*> GetAllTerritories() const { return AllTerritories; }

	UFUNCTION(BlueprintPure, Category = "Territories")
	TArray<ATDITerritoryBase*> GetCapturedTerritories() const;

	UFUNCTION(BlueprintPure, Category = "Territories")
	TArray<ATDITerritoryBase*> GetWildTerritories() const;

	UFUNCTION(BlueprintPure, Category = "Territories")
	int32 GetCapturedCount() const;

	UFUNCTION(BlueprintPure, Category = "Territories")
	ATDITerritoryBase* FindTerritoryByID(const FString& TerritoryID) const;

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Territories|Events")
	FOnTerritoryStateChanged OnTerritoryStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Territories|Events")
	FOnTerritoryRegistered OnTerritoryRegistered;

	// ---- Serialization ----
	TArray<FTDITerritorySaveData> SerializeStates() const;
	void DeserializeStates(const TArray<FTDITerritorySaveData>& SaveData);

private:
	UPROPERTY()
	TArray<TObjectPtr<ATDITerritoryBase>> AllTerritories;
};
