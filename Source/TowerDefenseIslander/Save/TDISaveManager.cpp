#include "TDISaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/TDIResourceSubsystem.h"
#include "Subsystems/TDITerritorySubsystem.h"
#include "Subsystems/TDILogisticsSubsystem.h"
#include "Research/TDIResearchManager.h"
#include "Castle/TDICastle.h"
#include "Waves/TDIWaveManager.h"
#include "Logistics/TDIRoad.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UTDISaveManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTDISaveManager::SaveGame(int32 SlotIndex)
{
	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
	{
		OnSaveCompleted.Broadcast(false);
		return;
	}

	UTDISaveGame* SaveObj = Cast<UTDISaveGame>(
		UGameplayStatics::CreateSaveGameObject(UTDISaveGame::StaticClass()));
	if (!SaveObj)
	{
		OnSaveCompleted.Broadcast(false);
		return;
	}

	SaveObj->SlotIndex = SlotIndex;
	SaveObj->SaveData.SaveDate = FDateTime::Now().ToString();
	SaveObj->SaveData.MapName = World->GetMapName();

	CollectWorldState(SaveObj->SaveData, World);

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveObj,
		UTDISaveGame::GetSlotName(SlotIndex), 0);

	OnSaveCompleted.Broadcast(bSuccess);
}

bool UTDISaveManager::DoesSaveExist(int32 SlotIndex) const
{
	return UGameplayStatics::DoesSaveGameExist(UTDISaveGame::GetSlotName(SlotIndex), 0);
}

void UTDISaveManager::DeleteSave(int32 SlotIndex)
{
	UGameplayStatics::DeleteGameInSlot(UTDISaveGame::GetSlotName(SlotIndex), 0);
}

void UTDISaveManager::LoadGame(int32 SlotIndex)
{
	const FString SlotName = UTDISaveGame::GetSlotName(SlotIndex);

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		OnLoadCompleted.Broadcast(false);
		return;
	}

	PendingSave = Cast<UTDISaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	OnLoadCompleted.Broadcast(PendingSave != nullptr);
}

void UTDISaveManager::ApplySaveToCurrentWorld()
{
	if (!PendingSave) return;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World) return;

	RestoreWorldState(PendingSave->SaveData, World);
	PendingSave = nullptr;
}

void UTDISaveManager::StartAutoSave(float IntervalSeconds)
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoSaveTimer,
			[this]() { SaveGame(AutoSaveSlot); },
			IntervalSeconds, true);
	}
}

void UTDISaveManager::StopAutoSave()
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimer);
	}
}

void UTDISaveManager::CollectWorldState(FTDIGameSaveData& OutData, UWorld* World) const
{
	// Resources
	if (UTDIResourceSubsystem* Resources = World->GetSubsystem<UTDIResourceSubsystem>())
	{
		OutData.ResourceAmounts = Resources->SerializeToStringMap();
	}

	// Territories
	if (UTDITerritorySubsystem* Territories = World->GetSubsystem<UTDITerritorySubsystem>())
	{
		OutData.TerritoryStates = Territories->SerializeStates();
	}

	// Roads
	if (UTDILogisticsSubsystem* Logistics = World->GetSubsystem<UTDILogisticsSubsystem>())
	{
		OutData.RoadStates = Logistics->SerializeRoads();
	}

	// Castle
	if (ATDICastle* Castle = Cast<ATDICastle>(
		UGameplayStatics::GetActorOfClass(World, ATDICastle::StaticClass())))
	{
		OutData.CastleHealth = Castle->GetCurrentHealth();
		OutData.CastleLevel = Castle->GetCastleLevel();
	}

	// Wave
	if (ATDIWaveManager* WM = Cast<ATDIWaveManager>(
		UGameplayStatics::GetActorOfClass(World, ATDIWaveManager::StaticClass())))
	{
		OutData.CurrentWaveNumber = WM->GetCurrentWaveNumber();
		OutData.PreparationTimeRemaining = WM->GetPreparationTimeRemaining();
	}

	// Research
	if (ATDIResearchManager* RM = Cast<ATDIResearchManager>(
		UGameplayStatics::GetActorOfClass(World, ATDIResearchManager::StaticClass())))
	{
		OutData.UnlockedResearchTags = RM->SerializeUnlocks();
	}
}

void UTDISaveManager::RestoreWorldState(const FTDIGameSaveData& Data, UWorld* World)
{
	// Resources
	if (UTDIResourceSubsystem* Resources = World->GetSubsystem<UTDIResourceSubsystem>())
	{
		Resources->DeserializeFromStringMap(Data.ResourceAmounts);
	}

	// Territories
	if (UTDITerritorySubsystem* Territories = World->GetSubsystem<UTDITerritorySubsystem>())
	{
		Territories->DeserializeStates(Data.TerritoryStates);
	}

	// Research
	if (ATDIResearchManager* RM = Cast<ATDIResearchManager>(
		UGameplayStatics::GetActorOfClass(World, ATDIResearchManager::StaticClass())))
	{
		RM->DeserializeUnlocks(Data.UnlockedResearchTags);
	}

	// Roads
	TArray<AActor*> RoadActors;
	UGameplayStatics::GetAllActorsOfClass(World, ATDIRoad::StaticClass(), RoadActors);
	for (AActor* A : RoadActors)
	{
		if (ATDIRoad* Road = Cast<ATDIRoad>(A))
		{
			for (const FTDIRoadSaveData& RoadSave : Data.RoadStates)
			{
				if (Road->GetName() == RoadSave.RoadID)
				{
					Road->LoadFromSaveData(RoadSave);
					break;
				}
			}
		}
	}
}
