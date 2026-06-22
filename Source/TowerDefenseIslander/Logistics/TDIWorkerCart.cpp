#include "TDIWorkerCart.h"
#include "TDIRoad.h"
#include "Territory/TDIOutpost.h"
#include "Castle/TDICastle.h"
#include "Subsystems/TDIResourceSubsystem.h"
#include "Subsystems/TDILogisticsSubsystem.h"
#include "PaperSpriteComponent.h"
#include "Components/SplineComponent.h"

ATDIWorkerCart::ATDIWorkerCart()
{
	PrimaryActorTick.bCanEverTick = true;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SetRootComponent(SpriteComponent);
}

void ATDIWorkerCart::BeginPlay()
{
	Super::BeginPlay();
}

void ATDIWorkerCart::Initialize(ATDIRoad* Road, ATDIOutpost* Outpost, ATDICastle* Castle)
{
	AssignedRoad = Road;
	AssignedOutpost = Outpost;
	DestinationCastle = Castle;

	if (Road && Road->GetRoadTier() != ERoadTier::Dirt)
	{
		// Roads store speed multipliers per tier; fetch index
		const int32 TierIdx = static_cast<int32>(Road->GetRoadTier());
		// Default multiplier, designer can override via SetTravelSpeedMultiplier
	}

	SetState(ECartState::TravelingToTerritory);
	SplineProgress = 1.0f;  // Start at castle end, travel toward territory
	bTravelingToCastle = false;
}

void ATDIWorkerCart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!AssignedRoad || AssignedRoad->GetRoadState() == ERoadState::Destroyed)
	{
		SetState(ECartState::Disabled);
		return;
	}

	switch (CartState)
	{
	case ECartState::TravelingToTerritory:
	case ECartState::TravelingToCastle:
		UpdateMovement(DeltaTime);
		break;

	case ECartState::Loading:
	case ECartState::Unloading:
		ActionTimer -= DeltaTime;
		if (ActionTimer <= 0.0f)
		{
			if (CartState == ECartState::Loading) OnLoadComplete();
			else OnUnloadComplete();
		}
		break;

	default:
		break;
	}
}

void ATDIWorkerCart::Destroyed()
{
	if (UTDILogisticsSubsystem* Logistics = GetWorld()->GetSubsystem<UTDILogisticsSubsystem>())
	{
		Logistics->OnCartDestroyed(this);
	}
	Super::Destroyed();
}

void ATDIWorkerCart::SetCartCapacity(float NewCapacity)
{
	CartCapacity = FMath::Max(NewCapacity, 10.0f);
}

void ATDIWorkerCart::SetTravelSpeedMultiplier(float Multiplier)
{
	TravelSpeedMultiplier = FMath::Max(Multiplier, 0.1f);
}

float ATDIWorkerCart::GetLoadPercent() const
{
	float Total = 0.0f;
	for (const FTDIResourceAmount& R : Cargo) Total += R.Amount;
	return CartCapacity > 0.0f ? FMath::Clamp(Total / CartCapacity, 0.0f, 1.0f) : 0.0f;
}

void ATDIWorkerCart::SetState(ECartState NewState)
{
	CartState = NewState;
}

void ATDIWorkerCart::UpdateMovement(float DeltaTime)
{
	if (!AssignedRoad) return;

	USplineComponent* Spline = AssignedRoad->GetSpline();
	if (!Spline) return;

	const float SplineLength = Spline->GetSplineLength();
	if (SplineLength <= 0.0f) return;

	const float Speed = BaseTravelSpeed * TravelSpeedMultiplier;
	const float ProgressDelta = (Speed * DeltaTime) / SplineLength;

	if (bTravelingToCastle)
	{
		SplineProgress += ProgressDelta;
		if (SplineProgress >= 1.0f)
		{
			SplineProgress = 1.0f;
			StartUnloading();
		}
	}
	else
	{
		SplineProgress -= ProgressDelta;
		if (SplineProgress <= 0.0f)
		{
			SplineProgress = 0.0f;
			StartLoading();
		}
	}

	// Update actor location along spline
	const float Distance = SplineProgress * SplineLength;
	FVector NewLoc = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	NewLoc.Z = GetActorLocation().Z;

	FVector Dir = Spline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	if (!bTravelingToCastle) Dir = -Dir;

	SetActorLocation(NewLoc);
	if (!Dir.IsNearlyZero()) SetActorRotation(Dir.Rotation());
}

void ATDIWorkerCart::StartLoading()
{
	if (!AssignedOutpost || !AssignedOutpost->IsActive())
	{
		// Nothing to load, head back empty
		bTravelingToCastle = true;
		SetState(ECartState::TravelingToCastle);
		return;
	}
	SetState(ECartState::Loading);
	ActionTimer = LoadUnloadTime;
}

void ATDIWorkerCart::OnLoadComplete()
{
	if (AssignedOutpost)
	{
		Cargo = AssignedOutpost->PickupResources(CartCapacity);
	}
	bTravelingToCastle = true;
	SetState(ECartState::TravelingToCastle);
}

void ATDIWorkerCart::StartUnloading()
{
	SetState(ECartState::Unloading);
	ActionTimer = LoadUnloadTime;
}

void ATDIWorkerCart::OnUnloadComplete()
{
	// Deposit cargo into the resource subsystem
	if (UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>())
	{
		for (const FTDIResourceAmount& Item : Cargo)
		{
			Resources->AddResource(Item.ResourceType, Item.Amount);
		}
	}
	Cargo.Empty();

	// Head back to territory
	bTravelingToCastle = false;
	SetState(ECartState::TravelingToTerritory);
}
