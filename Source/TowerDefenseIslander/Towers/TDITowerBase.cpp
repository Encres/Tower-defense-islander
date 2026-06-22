#include "TDITowerBase.h"
#include "TDIProjectileBase.h"
#include "Enemies/TDIEnemyBase.h"
#include "Data/TDITowerData.h"
#include "Subsystems/TDIResourceSubsystem.h"
#include "Research/TDIResearchManager.h"
#include "PaperSpriteComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

ATDITowerBase::ATDITowerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SetRootComponent(SpriteComponent);

	RangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RangeSphere"));
	RangeSphere->SetupAttachment(RootComponent);
	RangeSphere->SetSphereRadius(400.0f);
	RangeSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RangeSphere->SetGenerateOverlapEvents(true);
}

void ATDITowerBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	ApplyTierStats();

	RangeSphere->OnComponentBeginOverlap.AddDynamic(this, &ATDITowerBase::OnRangeBeginOverlap);
	RangeSphere->OnComponentEndOverlap.AddDynamic(this, &ATDITowerBase::OnRangeEndOverlap);
}

void ATDITowerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGhost) return;

	FireCooldown -= DeltaTime;
	if (FireCooldown <= 0.0f)
	{
		if (ATDIEnemyBase* Target = SelectTarget())
		{
			Fire(Target);

			const FTDITowerTierStats* Stats = TowerData ? TowerData->GetTierStats(CurrentTier) : nullptr;
			const float FireRate = Stats ? Stats->FireRate : 1.0f;
			FireCooldown = FireRate > 0.0f ? 1.0f / FireRate : 1.0f;
		}
	}
}

float ATDITowerBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth = FMath::Max(CurrentHealth - Applied, 0.0f);
	if (CurrentHealth <= 0.0f) Destroy();
	return Applied;
}

void ATDITowerBase::SetGhostMode(bool bGhost)
{
	bIsGhost = bGhost;
	SpriteComponent->SetOpacity(bGhost ? 0.5f : 1.0f);
	SetActorEnableCollision(!bGhost);
}

void ATDITowerBase::OnPlaced_Implementation()
{
	SetGhostMode(false);
}

bool ATDITowerBase::CanPlaceHere_Implementation() const
{
	// Disallow stacking on another tower
	TArray<AActor*> Overlapping;
	GetOverlappingActors(Overlapping, ATDITowerBase::StaticClass());
	return Overlapping.Num() == 0;
}

bool ATDITowerBase::TryUpgrade()
{
	if (!CanUpgrade()) return false;

	const FTDITowerTierStats* Stats = TowerData->GetTierStats(CurrentTier);
	if (!Stats) return false;

	UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>();
	if (!Resources || !Resources->SpendResources(Stats->UpgradeCost)) return false;

	CurrentTier = static_cast<ETowerTier>(static_cast<int32>(CurrentTier) + 1);
	ApplyTierStats();
	OnTierChanged(CurrentTier);
	OnTowerUpgraded.Broadcast(static_cast<int32>(CurrentTier));
	return true;
}

bool ATDITowerBase::CanUpgrade() const
{
	if (!TowerData) return false;
	const int32 NextTierIdx = static_cast<int32>(CurrentTier) + 1;
	if (NextTierIdx > static_cast<int32>(ETowerTier::Tier4)) return false;

	// Check research requirement for next tier
	const FTDITowerTierStats* NextStats = TowerData->GetTierStats(static_cast<ETowerTier>(NextTierIdx));
	if (!NextStats) return false;

	if (NextStats->RequiredResearch.IsValid())
	{
		ATDIResearchManager* RM = Cast<ATDIResearchManager>(
			UGameplayStatics::GetActorOfClass(this, ATDIResearchManager::StaticClass()));
		if (!RM || !RM->IsUnlocked(NextStats->RequiredResearch)) return false;
	}
	return true;
}

TArray<FTDIResourceCost> ATDITowerBase::GetUpgradeCost() const
{
	if (!TowerData) return {};
	const FTDITowerTierStats* Stats = TowerData->GetTierStats(CurrentTier);
	return Stats ? Stats->UpgradeCost : TArray<FTDIResourceCost>{};
}

float ATDITowerBase::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

ATDIEnemyBase* ATDITowerBase::SelectTarget_Implementation() const
{
	if (!TowerData) return nullptr;

	ATDIEnemyBase* Best = nullptr;
	float BestValue = -1.0f;

	for (const TWeakObjectPtr<ATDIEnemyBase>& Weak : EnemiesInRange)
	{
		ATDIEnemyBase* Enemy = Weak.Get();
		if (!Enemy || Enemy->IsDead()) continue;

		// Prioritize furthest along path by default; invert for "nearest" mode
		const float Value = TowerData->bPrioritizeFurthest
			? Enemy->GetPathProgress()
			: -Enemy->GetPathProgress();

		if (Value > BestValue)
		{
			BestValue = Value;
			Best = Enemy;
		}
	}

	return Best;
}

void ATDITowerBase::Fire_Implementation(ATDIEnemyBase* Target)
{
	if (!Target || !TowerData) return;

	const FTDITowerTierStats* Stats = TowerData->GetTierStats(CurrentTier);
	if (!Stats) return;

	// Compute effective damage (bonus vs specific enemy types)
	float Damage = Stats->Damage;
	if (TowerData->BonusDamageAgainst.IsValid() &&
		Target->GetEnemyTags().HasAny(TowerData->BonusDamageAgainst))
	{
		Damage *= TowerData->BonusDamageMultiplier;
	}

	if (TowerData->ProjectileClass)
	{
		// Spawn a projectile
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ATDIProjectileBase* Proj = GetWorld()->SpawnActor<ATDIProjectileBase>(
			TowerData->ProjectileClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
		if (Proj)
		{
			Proj->Initialize(Target, Damage, Stats->SplashRadius, this);
			Proj->ApplyEffect(Stats->SlowAmount, Stats->DotDamagePerSec, Stats->DotDuration);
		}
	}
	else
	{
		// Instant hit
		FDamageEvent DmgEvent;
		Target->TakeDamage(Damage, DmgEvent, nullptr, this);
		ApplyStatusEffect(Target, *Stats);
	}
}

void ATDITowerBase::ApplyStatusEffect_Implementation(ATDIEnemyBase* Target, const FTDITowerTierStats& Stats)
{
	if (!Target) return;

	FTDIStatusEffect Effect;
	Effect.SlowMultiplier = 1.0f - Stats.SlowAmount;
	Effect.DotDamagePerSec = Stats.DotDamagePerSec;
	Effect.RemainingDuration = Stats.DotDuration;

	if (TowerData)
	{
		// Tag the effect source so immunities can be checked
		if (TowerData->TowerType == ETowerType::Frost)   Effect.EffectSource = TDITag_Effect_Slow;
		if (TowerData->TowerType == ETowerType::Fire)    Effect.EffectSource = TDITag_Effect_Burn;
		if (TowerData->TowerType == ETowerType::Poison)  Effect.EffectSource = TDITag_Effect_Poison;
	}

	if (Effect.SlowMultiplier < 1.0f || Effect.DotDamagePerSec > 0.0f)
	{
		Target->ApplyStatusEffect(Effect);
	}
}

void ATDITowerBase::ApplyTierStats()
{
	if (!TowerData) return;
	const FTDITowerTierStats* Stats = TowerData->GetTierStats(CurrentTier);
	if (Stats)
	{
		RangeSphere->SetSphereRadius(Stats->Range);
	}
}

void ATDITowerBase::OnRangeBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (ATDIEnemyBase* Enemy = Cast<ATDIEnemyBase>(OtherActor))
	{
		EnemiesInRange.AddUnique(Enemy);
	}
}

void ATDITowerBase::OnRangeEndOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32)
{
	if (ATDIEnemyBase* Enemy = Cast<ATDIEnemyBase>(OtherActor))
	{
		EnemiesInRange.Remove(Enemy);
	}
}
