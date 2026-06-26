#include "TDIProjectileBase.h"
#include "Enemies/TDIEnemyBase.h"
#include "PaperSpriteComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

ATDIProjectileBase::ATDIProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SetRootComponent(SpriteComponent);
}

void ATDIProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

void ATDIProjectileBase::Initialize(ATDIEnemyBase* InTarget, float InDamage,
	float InSplashRadius, AActor* InTowerInstigatorActor)
{
	Target = InTarget;
	Damage = InDamage;
	SplashRadius = InSplashRadius;
	TowerInstigator = InTowerInstigatorActor;
}

void ATDIProjectileBase::ApplyEffect(float SlowAmount, float DotDamage, float DotDuration)
{
	PendingSlowAmount = SlowAmount;
	PendingDotDamage = DotDamage;
	PendingDotDuration = DotDuration;
}

void ATDIProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LifeTimer += DeltaTime;
	if (LifeTimer >= MaxLifetime)
	{
		Destroy();
		return;
	}

	ATDIEnemyBase* LiveTarget = Target.Get();
	if (!LiveTarget || LiveTarget->IsDead())
	{
		Destroy();
		return;
	}

	// Homing movement toward target
	const FVector TargetLoc = LiveTarget->GetActorLocation();
	const FVector CurrentLoc = GetActorLocation();
	const FVector Dir = (TargetLoc - CurrentLoc).GetSafeNormal();
	const float StepSize = Speed * DeltaTime;

	if (FVector::Dist(CurrentLoc, TargetLoc) <= StepSize)
	{
		// Reached target
		OnHit(LiveTarget);
		DetonateAtLocation(TargetLoc);
		Destroy();
	}
	else
	{
		SetActorLocation(CurrentLoc + Dir * StepSize);
		if (!Dir.IsNearlyZero()) SetActorRotation(Dir.Rotation());
	}
}

void ATDIProjectileBase::OnHit_Implementation(ATDIEnemyBase* HitEnemy)
{
	// Base: direct damage + status effect
	if (HitEnemy)
	{
		FDamageEvent DmgEvent;
		HitEnemy->TakeDamage(Damage, DmgEvent, nullptr, TowerInstigator);

		if (PendingSlowAmount > 0.0f || PendingDotDamage > 0.0f)
		{
			FTDIStatusEffect Effect;
			Effect.SlowMultiplier = 1.0f - PendingSlowAmount;
			Effect.DotDamagePerSec = PendingDotDamage;
			Effect.RemainingDuration = PendingDotDuration;
			HitEnemy->ApplyStatusEffect(Effect);
		}
	}
}

void ATDIProjectileBase::DetonateAtLocation(const FVector& Location)
{
	if (SplashRadius <= 0.0f) return;

	// Find all enemies within splash radius
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SplashRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->SweepMultiByObjectType(Hits, Location, Location, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), Sphere, Params);

	for (const FHitResult& Hit : Hits)
	{
		if (ATDIEnemyBase* Enemy = Cast<ATDIEnemyBase>(Hit.GetActor()))
		{
			// Splash damage falls off with distance
			const float Dist = FVector::Dist(Location, Enemy->GetActorLocation());
			const float Falloff = 1.0f - FMath::Clamp(Dist / SplashRadius, 0.0f, 1.0f);
			const float SplashDmg = Damage * Falloff * 0.5f;

			FDamageEvent DmgEvent;
			Enemy->TakeDamage(SplashDmg, DmgEvent, nullptr, TowerInstigator);
		}
	}
}
