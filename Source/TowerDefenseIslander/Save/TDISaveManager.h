#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TDISaveGame.h"
#include "TDISaveManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleted, bool, bSuccess);

// GameInstance subsystem: persists across level loads.
// Collects state from all world subsystems and actors, serializes to UTDISaveGame.
UCLASS()
class TOWERDEFENSEISLANDER_API UTDISaveManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ---- Save ----
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGame(int32 SlotIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DoesSaveExist(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	void DeleteSave(int32 SlotIndex);

	// ---- Load ----
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadGame(int32 SlotIndex = 0);

	// Apply a loaded save to the current world (call after level is loaded)
	UFUNCTION(BlueprintCallable, Category = "Save")
	void ApplySaveToCurrentWorld();

	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasPendingSave() const { return PendingSave != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Save")
	UTDISaveGame* GetLastLoadedSave() const { return PendingSave; }

	// ---- Auto-save ----
	UFUNCTION(BlueprintCallable, Category = "Save")
	void StartAutoSave(float IntervalSeconds = 60.0f);

	UFUNCTION(BlueprintCallable, Category = "Save")
	void StopAutoSave();

	// ---- Events ----
	UPROPERTY(BlueprintAssignable, Category = "Save|Events")
	FOnSaveCompleted OnSaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Save|Events")
	FOnLoadCompleted OnLoadCompleted;

	static constexpr int32 MaxSaveSlots = 3;
	static constexpr int32 AutoSaveSlot = 99;

private:
	UPROPERTY()
	TObjectPtr<UTDISaveGame> PendingSave;

	FTimerHandle AutoSaveTimer;

	void CollectWorldState(FTDIGameSaveData& OutData, UWorld* World) const;
	void RestoreWorldState(const FTDIGameSaveData& Data, UWorld* World);
};
