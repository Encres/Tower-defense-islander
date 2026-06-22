#include "TDICastle.h"
#include "PaperSpriteComponent.h"
#include "Components/BoxComponent.h"
#include "Subsystems/TDIResourceSubsystem.h"
#include "Subsystems/TDILogisticsSubsystem.h"
#include "Kismet/GameplayStatics.h"

ATDICastle::ATDICastle()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetBoxExtent(FVector(64.0f, 64.0f, 32.0f));
	CollisionBox->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(RootComponent);

	ResourceDropOffPoint = CreateDefaultSubobject<USceneComponent>(TEXT("DropOffPoint"));
	ResourceDropOffPoint->SetupAttachment(RootComponent);
	ResourceDropOffPoint->SetRelativeLocation(FVector(-80.0f, 0.0f, 0.0f));
}

void ATDICastle::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	// Register with logistics subsystem so roads know where to deliver resources
	if (UTDILogisticsSubsystem* Logistics = GetWorld()->GetSubsystem<UTDILogisticsSubsystem>())
	{
		Logistics->RegisterCastle(this);
	}
}

float ATDICastle::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth = FMath::Max(CurrentHealth - Applied, 0.0f);

	OnCastleHealthChanged.Broadcast(GetHealthPercent());

	if (CurrentHealth <= 0.0f)
	{
		OnCastleDestroyed.Broadcast();
	}

	return Applied;
}

FVector ATDICastle::GetResourceDropOffLocation() const
{
	return ResourceDropOffPoint->GetComponentLocation();
}

float ATDICastle::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

bool ATDICastle::TryUpgradeCastle()
{
	if (CastleLevel >= MaxCastleLevel) return false;
	if (!UpgradeCosts.IsValidIndex(CastleLevel - 1)) return false;

	UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>();
	if (!Resources) return false;

	// Level 1→2 uses UpgradeCosts[0], Level 2→3 uses UpgradeCosts[1]
	TArray<FTDIResourceAmount> Cost;
	Cost.Add(UpgradeCosts[CastleLevel - 1]);
	if (!Resources->SpendResources(Cost)) return false;

	CastleLevel++;
	ApplyLevelUpgrade();
	return true;
}

void ATDICastle::ApplyLevelUpgrade()
{
	// Scale max health per level
	MaxHealth *= 1.5f;
	CurrentHealth = MaxHealth;

	// Expand storage capacity
	UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>();
	if (Resources)
	{
		const float NewCapacity = 500.0f * FMath::Pow(3.0f, static_cast<float>(CastleLevel - 1));
		const float GoldCapacity = 9999.0f;

		auto SetCap = [&](FGameplayTag Tag, float Cap)
		{
			Resources->SetStorageCapacity(Tag, Cap);
		};

		SetCap(TDITag_Resource_Wood,        NewCapacity);
		SetCap(TDITag_Resource_Leather,     NewCapacity);
		SetCap(TDITag_Resource_Bone,        NewCapacity);
		SetCap(TDITag_Resource_DarkEssence, NewCapacity);
		SetCap(TDITag_Resource_Silk,        NewCapacity);
		SetCap(TDITag_Resource_Venom,       NewCapacity);
		SetCap(TDITag_Resource_Stone,       NewCapacity);
		SetCap(TDITag_Resource_Iron,        NewCapacity);
		SetCap(TDITag_Resource_DragonScale, NewCapacity);
		SetCap(TDITag_Resource_Crystal,     NewCapacity);
		SetCap(TDITag_Resource_Gold,        GoldCapacity);
	}

	// Expand carts per territory
	if (UTDILogisticsSubsystem* Logistics = GetWorld()->GetSubsystem<UTDILogisticsSubsystem>())
	{
		const int32 NewCarts = 2 * CastleLevel;
		Logistics->SetBaseCartsPerTerritory(NewCarts);
	}
}
