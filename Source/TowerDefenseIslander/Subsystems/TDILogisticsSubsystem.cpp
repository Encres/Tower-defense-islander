#include "TDILogisticsSubsystem.h"
#include "Logistics/TDIRoad.h"
#include "Logistics/TDIWorkerCart.h"
#include "Castle/TDICastle.h"
#include "Territory/TDITerritoryBase.h"
#include "Territory/TDIOutpost.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UTDILogisticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTDILogisticsSubsystem::RegisterRoad(ATDIRoad* Road)
{
	if (!Road || AllRoads.Contains(Road)) return;
	AllRoads.Add(Road);
	ValidateNetwork();
}

void UTDILogisticsSubsystem::UnregisterRoad(ATDIRoad* Road)
{
	AllRoads.Remove(Road);
	ValidateNetwork();
}

void UTDILogisticsSubsystem::RegisterCastle(ATDICastle* Castle)
{
	CastleActor = Castle;
	ValidateNetwork();
}

void UTDILogisticsSubsystem::ValidateNetwork()
{
	if (!CastleActor) return;

	TSet<ATDIRoad*> ReachableRoads;
	BFSFromCastle(ReachableRoads);

	// Update each territory's connectivity
	TSet<ATDITerritoryBase*> Territories;
	for (TObjectPtr<ATDIRoad> Road : AllRoads)
	{
		if (!Road) continue;
		if (ATDITerritoryBase* T = Road->GetConnectedTerritory())
		{
			Territories.Add(T);
		}
	}

	bool bAnyBroken = false;
	for (ATDITerritoryBase* Territory : Territories)
	{
		// Territory is connected if any road linking to it is reachable
		bool bConnected = false;
		for (TObjectPtr<ATDIRoad> Road : AllRoads)
		{
			if (Road && Road->GetConnectedTerritory() == Territory
				&& ReachableRoads.Contains(Road.Get()))
			{
				bConnected = true;
				break;
			}
		}

		const bool bWasConnected = ConnectivityMap.FindRef(Territory);
		if (bConnected != bWasConnected)
		{
			ConnectivityMap.Add(Territory, bConnected);
			OnRouteConnectivityChanged.Broadcast(Territory, bConnected);

			if (bConnected)
			{
				SpawnCartsForTerritory(Territory);
			}
			else
			{
				DespawnCartsForTerritory(Territory);
				bAnyBroken = true;
			}
		}
	}

	OnRoadNetworkChanged.Broadcast(bAnyBroken);
}

bool UTDILogisticsSubsystem::IsTerritoryConnected(ATDITerritoryBase* Territory) const
{
	return ConnectivityMap.FindRef(Territory);
}

TArray<ATDIRoad*> UTDILogisticsSubsystem::GetConnectedRoads() const
{
	TSet<ATDIRoad*> Reachable;
	BFSFromCastle(Reachable);
	return Reachable.Array();
}

void UTDILogisticsSubsystem::SpawnCartsForTerritory(ATDITerritoryBase* Territory)
{
	if (!Territory || !CastleActor) return;
	if (Territory->GetTerritoryState() != ETerritoryState::Captured) return;

	ATDIOutpost* Outpost = Territory->GetOutpost();
	if (!Outpost) return;

	// Find the road for this territory
	ATDIRoad* TerritoryRoad = nullptr;
	for (TObjectPtr<ATDIRoad> Road : AllRoads)
	{
		if (Road && Road->GetConnectedTerritory() == Territory)
		{
			TerritoryRoad = Road;
			break;
		}
	}
	if (!TerritoryRoad) return;

	TArray<TObjectPtr<ATDIWorkerCart>>& Carts = CartsByTerritory.FindOrAdd(Territory);
	const int32 DesiredCount = BaseCartsPerTerritory;

	while (Carts.Num() < DesiredCount)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector SpawnLoc = Outpost->GetActorLocation();
		ATDIWorkerCart* Cart = GetWorld()->SpawnActor<ATDIWorkerCart>(
			ATDIWorkerCart::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);

		if (Cart)
		{
			Cart->Initialize(TerritoryRoad, Outpost, CastleActor);
			Carts.Add(Cart);
		}
	}
}

void UTDILogisticsSubsystem::DespawnCartsForTerritory(ATDITerritoryBase* Territory)
{
	if (TArray<TObjectPtr<ATDIWorkerCart>>* Carts = CartsByTerritory.Find(Territory))
	{
		for (TObjectPtr<ATDIWorkerCart> Cart : *Carts)
		{
			if (Cart) Cart->Destroy();
		}
		Carts->Empty();
	}
}

void UTDILogisticsSubsystem::OnCartDestroyed(ATDIWorkerCart* Cart)
{
	// Find which territory this cart belonged to and schedule respawn
	for (auto& Pair : CartsByTerritory)
	{
		if (Pair.Value.Contains(Cart))
		{
			Pair.Value.Remove(Cart);

			ATDITerritoryBase* Territory = Pair.Key;
			FTimerHandle RespawnTimer;
			GetWorld()->GetTimerManager().SetTimer(RespawnTimer,
				[this, Territory]()
				{
					if (Territory && IsTerritoryConnected(Territory))
					{
						SpawnCartsForTerritory(Territory);
					}
				},
				CartRespawnDelay, false);
			break;
		}
	}
}

void UTDILogisticsSubsystem::SetBaseCartsPerTerritory(int32 NewCount)
{
	BaseCartsPerTerritory = FMath::Max(NewCount, 1);

	// Spawn additional carts for connected territories if count increased
	for (auto& Pair : ConnectivityMap)
	{
		if (Pair.Value)
		{
			SpawnCartsForTerritory(Pair.Key);
		}
	}
}

void UTDILogisticsSubsystem::BFSFromCastle(TSet<ATDIRoad*>& OutReachable) const
{
	if (!CastleActor) return;

	// Build adjacency: road → connected roads (roads that share an endpoint)
	// Each road stores its FromActor and ToActor; roads sharing an actor are adjacent
	TQueue<ATDIRoad*> Queue;
	TSet<ATDIRoad*> Visited;

	// Seed: all roads that connect directly to the castle
	for (TObjectPtr<ATDIRoad> Road : AllRoads)
	{
		if (Road && Road->IsConnectedToCastle())
		{
			Queue.Enqueue(Road.Get());
			Visited.Add(Road.Get());
		}
	}

	while (!Queue.IsEmpty())
	{
		ATDIRoad* Current;
		Queue.Dequeue(Current);
		OutReachable.Add(Current);

		// Find roads that share an endpoint with Current
		for (TObjectPtr<ATDIRoad> Other : AllRoads)
		{
			if (!Other || Visited.Contains(Other.Get())) continue;
			if (Current->SharesEndpointWith(Other.Get()))
			{
				Visited.Add(Other.Get());
				Queue.Enqueue(Other.Get());
			}
		}
	}
}

TArray<FTDIRoadSaveData> UTDILogisticsSubsystem::SerializeRoads() const
{
	TArray<FTDIRoadSaveData> Result;
	for (TObjectPtr<ATDIRoad> Road : AllRoads)
	{
		if (Road)
		{
			Result.Add(Road->GetSaveData());
		}
	}
	return Result;
}
