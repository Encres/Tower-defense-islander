#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/TDIDataTypes.h"
#include "TDISaveGame.generated.h"

USTRUCT()
struct FTDIGameSaveData
{
	GENERATED_BODY()

	// ---- Resources ----
	UPROPERTY()
	TMap<FString, float> ResourceAmounts;

	// ---- Wave ----
	UPROPERTY()
	int32 CurrentWaveNumber = 0;

	UPROPERTY()
	float PreparationTimeRemaining = 90.0f;

	// ---- Castle ----
	UPROPERTY()
	float CastleHealth = 1000.0f;

	UPROPERTY()
	int32 CastleLevel = 1;

	// ---- Territories ----
	UPROPERTY()
	TArray<FTDITerritorySaveData> TerritoryStates;

	// ---- Towers ----
	UPROPERTY()
	TArray<FTDITowerSaveData> PlacedTowers;

	// ---- Roads ----
	UPROPERTY()
	TArray<FTDIRoadSaveData> RoadStates;

	// ---- Research ----
	UPROPERTY()
	TArray<FString> UnlockedResearchTags;

	// ---- Meta ----
	UPROPERTY()
	FString SaveDate;

	UPROPERTY()
	int32 PlaytimeSeconds = 0;

	UPROPERTY()
	FString MapName;
};

UCLASS()
class TOWERDEFENSEISLANDER_API UTDISaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FTDIGameSaveData SaveData;

	UPROPERTY()
	int32 SlotIndex = 0;

	static const FString SaveSlotPrefix;
	static FString GetSlotName(int32 SlotIndex);
};
