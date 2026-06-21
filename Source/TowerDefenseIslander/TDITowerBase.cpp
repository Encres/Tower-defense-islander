#include "TDITowerBase.h"
#include "TDIEnemyBase.h"
#include "TDIGameMode.h"
#include "PaperSpriteComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

ATDITowerBase::ATDITowerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SetRootComponent(SpriteComponent);

	RangeCollider = CreateDefaultSubobject<USphereComponent>(TEXT("RangeCollider"));
	RangeCollider->SetupAttachment(RootComponent);
	RangeCollider->SetSphereRadius(Stats.Range);
	RangeCollider->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RangeCollider->SetGenerateOverlapEvents(true);
}

void ATDITowerBase::BeginPlay()
{
	Super::BeginPlay();
	RangeCollider->SetSphereRadius(Stats.Range);
	RangeCollider->OnComponentBeginOverlap.AddDynamic(this, &ATDITowerBase::OnRangeBeginOverlap);
	RangeCollider->OnComponentEndOverlap.AddDynamic(this, &ATDITowerBase::OnRangeEndOverlap);
}

void ATDITowerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGhost) return;

	FireCooldown -= DeltaTime;
	if (FireCooldown <= 0.0f)
	{
		if (ATDIEnemyBase* Target = FindTarget())
		{
			FireAtTarget(Target);
			FireCooldown = 1.0f / FMath::Max(Stats.FireRate, 0.01f);
		}
	}
}

void ATDITowerBase::OnPlaced_Implementation()
{
	SetGhostMode(false);
}

void ATDITowerBase::SetGhostMode(bool bGhost)
{
	bIsGhost = bGhost;
	if (SpriteComponent)
	{
		SpriteComponent->SetOpacity(bGhost ? 0.5f : 1.0f);
	}
	SetActorEnableCollision(!bGhost);
}

bool ATDITowerBase::CanPlaceHere_Implementation() const
{
	// Subclasses can refine — default checks only that nothing blocks the spot
	TArray<AActor*> Overlapping;
	GetOverlappingActors(Overlapping, ATDITowerBase::StaticClass());
	return Overlapping.Num() == 0;
}

bool ATDITowerBase::TryUpgrade()
{
	if (CurrentUpgradeLevel >= MaxUpgradeLevel) return false;

	ATDIGameMode* GM = Cast<ATDIGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GM || !GM->SpendGold(Stats.UpgradeCost)) return false;

	CurrentUpgradeLevel++;
	Stats.Damage *= 1.5f;
	Stats.Range  *= 1.2f;
	Stats.FireRate *= 1.2f;
	Stats.UpgradeCost = FMath::RoundToInt(Stats.UpgradeCost * 1.5f);
	RangeCollider->SetSphereRadius(Stats.Range);
	return true;
}

ATDIEnemyBase* ATDITowerBase::FindTarget() const
{
	ATDIEnemyBase* Best = nullptr;
	float BestProgress = -1.0f;

	for (const TWeakObjectPtr<ATDIEnemyBase>& Weak : EnemiesInRange)
	{
		if (ATDIEnemyBase* Enemy = Weak.Get())
		{
			float Progress = Enemy->GetPathProgress();
			if (Progress > BestProgress)
			{
				BestProgress = Progress;
				Best = Enemy;
			}
		}
	}
	return Best;
}

void ATDITowerBase::FireAtTarget_Implementation(ATDIEnemyBase* Target)
{
	// Base implementation deals instant damage; subclasses spawn projectiles
	if (Target)
	{
		Target->TakeDamage(Stats.Damage, FDamageEvent(), nullptr, this);
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
