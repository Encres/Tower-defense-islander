#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TDIDataTypes.h"
#include "TDITerritoryBase.generated.h"

class UPaperSpriteComponent;
class UCapsuleComponent;
class ATDIOutpost;
class ATDIMonsterSpawner;
class ATDIResourceNode;
class UTDITerritoryData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerritoryStateChangedLocal, ATDITerritoryBase*, Territory, ETerritoryState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCaptureProgressUpdated, float, Progress);

UCLASS(Abstract)
class TOWERDEFENSEISLANDER_API ATDITerritoryBase : public AActor
{
	GENERATED_BODY()

public:
	ATDITerritoryBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ---- State ----
	UFUNCTION(BlueprintPure, Category = "Territory")
	ETerritoryState GetTerritoryState() const { return TerritoryState; }

	UFUNCTION(BlueprintPure, Category = "Territory")
	float GetCaptureProgress() const { return CaptureProgress; }

	UFUNCTION(BlueprintPure, Category = "Territory")
	FString GetTerritoryID() const { return TerritoryID; }

	UFUNCTION(BlueprintPure, Category = "Territory")
	const UTDITerritoryData* GetTerritoryData() const { return TerritoryData; }

	UFUNCTION(BlueprintPure, Category = "Territory")
	ATDIOutpost* GetOutpost() const { return Outpost; }

	// ---- Actions ----
	UFUNCTION(BlueprintCallable, Category = "Territory")
	void BeginCapture();

	// Called externally when enemies damage the capture progress
	UFUNCTION(BlueprintCallable, Category = "Territory")
	void InterruptCapture(float ProgressSetback);

	// Build the player's outpost at this territory (called after capture)
	UFUNCTION(BlueprintCallable, Category = "Territory")
	bool BuildOutpost();

	// ---- Save/Restore ----
	void RestoreState(ETerritoryState NewState, float Progress);

	// Called by outpost when it's destroyed — territory reverts to Wild
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Territory")
	void OnTerritoryLost();
	virtual void OnTerritoryLost_Implementation();

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Territory|Events")
	FOnTerritoryStateChangedLocal OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Territory|Events")
	FOnCaptureProgressUpdated OnCaptureProgressUpdated;

protected:
	// Unique ID matching a DA_Territory asset name — set in editor per instance
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Territory")
	FString TerritoryID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory")
	TObjectPtr<UTDITerritoryData> TerritoryData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory")
	TSubclassOf<ATDIOutpost> OutpostClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> TerritoryBounds;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Territory")
	void OnCaptureCompleted();
	virtual void OnCaptureCompleted_Implementation();

private:
	ETerritoryState TerritoryState = ETerritoryState::Wild;
	float CaptureProgress = 0.0f;   // 0–1
	bool bCapturing = false;

	UPROPERTY()
	TObjectPtr<ATDIOutpost> Outpost;

	UPROPERTY()
	TArray<TObjectPtr<ATDIMonsterSpawner>> Spawners;

	void ChangeState(ETerritoryState NewState);
};
