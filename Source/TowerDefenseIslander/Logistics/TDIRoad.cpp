#include "TDIRoad.h"
#include "Castle/TDICastle.h"
#include "Territory/TDITerritoryBase.h"
#include "Subsystems/TDILogisticsSubsystem.h"
#include "Subsystems/TDIResourceSubsystem.h"
#include "Components/SplineComponent.h"

ATDIRoad::ATDIRoad()
{
	PrimaryActorTick.bCanEverTick = false;

	RoadSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RoadSpline"));
	SetRootComponent(RoadSpline);
}

void ATDIRoad::BeginPlay()
{
	Super::BeginPlay();

	// Determine current max health from tier
	const int32 TierIdx = static_cast<int32>(RoadTier);
	CurrentHealth = MaxHealthPerTier.IsValidIndex(TierIdx) ? MaxHealthPerTier[TierIdx] : 100.0f;

	// Is this road directly attached to the castle?
	bConnectedToCastle = (NearCastle != nullptr);

	if (UTDILogisticsSubsystem* Logistics = GetWorld()->GetSubsystem<UTDILogisticsSubsystem>())
	{
		Logistics->RegisterRoad(this);
	}
}

void ATDIRoad::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTDILogisticsSubsystem* Logistics = GetWorld()->GetSubsystem<UTDILogisticsSubsystem>())
	{
		Logistics->UnregisterRoad(this);
	}
	Super::EndPlay(EndPlayReason);
}

float ATDIRoad::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth = FMath::Max(CurrentHealth - Applied, 0.0f);
	UpdateRoadState();
	return Applied;
}

void ATDIRoad::Repair(float Amount)
{
	const int32 TierIdx = static_cast<int32>(RoadTier);
	const float MaxHP = MaxHealthPerTier.IsValidIndex(TierIdx) ? MaxHealthPerTier[TierIdx] : 100.0f;

	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHP);
	UpdateRoadState();
}

bool ATDIRoad::TryUpgradeTier()
{
	const int32 NextTierIdx = static_cast<int32>(RoadTier) + 1;
	if (NextTierIdx >= static_cast<int32>(ERoadTier::Reinforced) + 1) return false;
	if (!UpgradeCostPerTier.IsValidIndex(static_cast<int32>(RoadTier))) return false;

	UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>();
	if (!Resources) return false;

	TArray<FTDIResourceAmount> Cost;
	Cost.Add(UpgradeCostPerTier[static_cast<int32>(RoadTier)]);
	if (!Resources->SpendResources(Cost)) return false;

	RoadTier = static_cast<ERoadTier>(NextTierIdx);
	const float NewMaxHP = MaxHealthPerTier.IsValidIndex(NextTierIdx) ? MaxHealthPerTier[NextTierIdx] : 200.0f;
	CurrentHealth = NewMaxHP;
	UpdateRoadState();
	return true;
}

bool ATDIRoad::SharesEndpointWith(const ATDIRoad* OtherRoad) const
{
	if (!OtherRoad) return false;

	// Two roads share an endpoint if one's NearRoad is the other,
	// or if both connect to the same territory / castle
	if (OtherRoad->NearRoad == this || NearRoad == OtherRoad) return true;
	if (ConnectedTerritory && ConnectedTerritory == OtherRoad->ConnectedTerritory) return true;
	return false;
}

float ATDIRoad::GetHealthPercent() const
{
	const int32 TierIdx = static_cast<int32>(RoadTier);
	const float MaxHP = MaxHealthPerTier.IsValidIndex(TierIdx) ? MaxHealthPerTier[TierIdx] : 100.0f;
	return MaxHP > 0.0f ? CurrentHealth / MaxHP : 0.0f;
}

void ATDIRoad::UpdateRoadState()
{
	const float HP = GetHealthPercent();
	ERoadState NewState;

	if (HP <= 0.0f)
	{
		NewState = ERoadState::Destroyed;
	}
	else if (HP < 0.5f)
	{
		NewState = ERoadState::Damaged;
	}
	else
	{
		NewState = ERoadState::Intact;
	}

	if (NewState != RoadState)
	{
		RoadState = NewState;
		OnRoadStateChanged.Broadcast(RoadState);

		if (RoadState == ERoadState::Destroyed)
		{
			if (UTDILogisticsSubsystem* Logistics = GetWorld()->GetSubsystem<UTDILogisticsSubsystem>())
			{
				Logistics->ValidateNetwork();
			}
		}
	}
}

FTDIRoadSaveData ATDIRoad::GetSaveData() const
{
	FTDIRoadSaveData Data;
	Data.RoadID = GetName();
	Data.Tier = RoadTier;
	Data.Health = CurrentHealth;
	return Data;
}

void ATDIRoad::LoadFromSaveData(const FTDIRoadSaveData& Data)
{
	RoadTier = Data.Tier;
	CurrentHealth = Data.Health;
	UpdateRoadState();
}
