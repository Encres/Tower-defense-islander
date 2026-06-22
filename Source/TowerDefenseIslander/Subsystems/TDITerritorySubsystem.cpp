#include "TDITerritorySubsystem.h"
#include "Territory/TDITerritoryBase.h"
#include "Subsystems/TDIResourceSubsystem.h"
#include "Data/TDITerritoryData.h"

void UTDITerritorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTDITerritorySubsystem::RegisterTerritory(ATDITerritoryBase* Territory)
{
	if (!Territory || AllTerritories.Contains(Territory)) return;
	AllTerritories.Add(Territory);
	OnTerritoryRegistered.Broadcast(Territory);
}

void UTDITerritorySubsystem::UnregisterTerritory(ATDITerritoryBase* Territory)
{
	AllTerritories.Remove(Territory);
}

bool UTDITerritorySubsystem::BeginCaptureAttempt(ATDITerritoryBase* Territory)
{
	if (!Territory) return false;
	if (Territory->GetTerritoryState() != ETerritoryState::Wild) return false;

	UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>();
	if (!Resources) return false;

	const UTDITerritoryData* Data = Territory->GetTerritoryData();
	if (!Data) return false;

	// Deduct capture cost
	TArray<FTDIResourceAmount> Cost;
	Cost.Add(FTDIResourceAmount(TDITag_Resource_Gold, static_cast<float>(Data->CaptureCostGold)));
	if (!Resources->SpendResources(Cost)) return false;

	Territory->BeginCapture();
	return true;
}

void UTDITerritorySubsystem::NotifyTerritoryStatChanged(ATDITerritoryBase* Territory, ETerritoryState NewState)
{
	OnTerritoryStateChanged.Broadcast(Territory, NewState);
}

TArray<ATDITerritoryBase*> UTDITerritorySubsystem::GetCapturedTerritories() const
{
	TArray<ATDITerritoryBase*> Result;
	for (TObjectPtr<ATDITerritoryBase> T : AllTerritories)
	{
		if (T && T->GetTerritoryState() == ETerritoryState::Captured)
		{
			Result.Add(T);
		}
	}
	return Result;
}

TArray<ATDITerritoryBase*> UTDITerritorySubsystem::GetWildTerritories() const
{
	TArray<ATDITerritoryBase*> Result;
	for (TObjectPtr<ATDITerritoryBase> T : AllTerritories)
	{
		if (T && T->GetTerritoryState() == ETerritoryState::Wild)
		{
			Result.Add(T);
		}
	}
	return Result;
}

int32 UTDITerritorySubsystem::GetCapturedCount() const
{
	return GetCapturedTerritories().Num();
}

ATDITerritoryBase* UTDITerritorySubsystem::FindTerritoryByID(const FString& TerritoryID) const
{
	for (TObjectPtr<ATDITerritoryBase> T : AllTerritories)
	{
		if (T && T->GetTerritoryID() == TerritoryID)
		{
			return T;
		}
	}
	return nullptr;
}

TArray<FTDITerritorySaveData> UTDITerritorySubsystem::SerializeStates() const
{
	TArray<FTDITerritorySaveData> Result;
	for (TObjectPtr<ATDITerritoryBase> T : AllTerritories)
	{
		if (!T) continue;
		FTDITerritorySaveData Save;
		Save.TerritoryID = T->GetTerritoryID();
		Save.State = T->GetTerritoryState();
		Save.CaptureProgress = T->GetCaptureProgress();
		Result.Add(Save);
	}
	return Result;
}

void UTDITerritorySubsystem::DeserializeStates(const TArray<FTDITerritorySaveData>& SaveData)
{
	for (const FTDITerritorySaveData& Save : SaveData)
	{
		if (ATDITerritoryBase* T = FindTerritoryByID(Save.TerritoryID))
		{
			T->RestoreState(Save.State, Save.CaptureProgress);
		}
	}
}
