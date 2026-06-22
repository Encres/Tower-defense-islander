#include "TDIEnemyBase.h"
#include "TDIEnemyAIController.h"
#include "Data/TDIEnemyData.h"
#include "Waves/TDIWaveManager.h"
#include "Subsystems/TDIResourceSubsystem.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ATDIEnemyBase::ATDIEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;
		Move->MaxWalkSpeed = 200.0f;
		Move->GravityScale = 0.0f;
		Move->MovementMode = MOVE_Flying;
	}

	bUseControllerRotationYaw = false;
	AIControllerClass = ATDIEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ATDIEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (EnemyData)
	{
		CurrentHealth = EnemyData->MaxHealth;
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->MaxWalkSpeed = EnemyData->MoveSpeed;
		}
	}
	else
	{
		CurrentHealth = 100.0f;
	}
}

void ATDIEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickStatusEffects(DeltaTime);

	// Sync movement speed with current status effect multiplier
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		const float BaseSpeed = EnemyData ? EnemyData->MoveSpeed : 200.0f;
		Move->MaxWalkSpeed = BaseSpeed * ComputeSpeedMultiplier();
	}
}

float ATDIEnemyBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;

	float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Apply armor reduction
	if (EnemyData)
	{
		Applied *= (1.0f - EnemyData->ArmorReduction);
	}

	CurrentHealth = FMath::Max(CurrentHealth - Applied, 0.0f);

	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		OnDeath();
	}

	return Applied;
}

void ATDIEnemyBase::ApplyStatusEffect(const FTDIStatusEffect& Effect)
{
	// Check immunity
	const FGameplayTagContainer& Tags = GetEnemyTags();
	if (EnemyData && EnemyData->ImmuneTags.HasTag(Effect.EffectSource)) return;

	// Stack or refresh existing effect of same source
	for (FTDIStatusEffect& Existing : ActiveEffects)
	{
		if (Existing.EffectSource == Effect.EffectSource)
		{
			Existing.RemainingDuration = FMath::Max(Existing.RemainingDuration, Effect.RemainingDuration);
			Existing.SlowMultiplier = FMath::Min(Existing.SlowMultiplier, Effect.SlowMultiplier);
			Existing.DotDamagePerSec = FMath::Max(Existing.DotDamagePerSec, Effect.DotDamagePerSec);
			return;
		}
	}
	ActiveEffects.Add(Effect);
}

bool ATDIEnemyBase::IsDead() const
{
	return bIsDead;
}

float ATDIEnemyBase::GetHealthPercent() const
{
	const float MaxHP = EnemyData ? EnemyData->MaxHealth : 100.0f;
	return MaxHP > 0.0f ? CurrentHealth / MaxHP : 0.0f;
}

float ATDIEnemyBase::GetCurrentSpeedMultiplier() const
{
	return ComputeSpeedMultiplier();
}

const FGameplayTagContainer& ATDIEnemyBase::GetEnemyTags() const
{
	static FGameplayTagContainer Empty;
	return EnemyData ? EnemyData->EnemyTags : Empty;
}

EEnemyObjective ATDIEnemyBase::GetPrimaryObjective() const
{
	return EnemyData ? EnemyData->PrimaryObjective : EEnemyObjective::AttackCastle;
}

void ATDIEnemyBase::OnDeath_Implementation()
{
	// Award gold
	if (EnemyData)
	{
		if (UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>())
		{
			Resources->AddResource(TDITag_Resource_Gold, static_cast<float>(EnemyData->GoldReward));
		}
	}

	// Notify wave manager
	if (ATDIWaveManager* WaveManager = Cast<ATDIWaveManager>(
		UGameplayStatics::GetActorOfClass(this, ATDIWaveManager::StaticClass())))
	{
		WaveManager->NotifyEnemyKilled();
	}

	OnEnemyDied.Broadcast(this);

	// Delay destroy to let death animation play
	SetActorEnableCollision(false);
	SetLifeSpan(1.5f);
}

void ATDIEnemyBase::TickStatusEffects(float DeltaTime)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		FTDIStatusEffect& Effect = ActiveEffects[i];
		Effect.RemainingDuration -= DeltaTime;

		if (Effect.DotDamagePerSec > 0.0f && !bIsDead)
		{
			const float DotDmg = Effect.DotDamagePerSec * DeltaTime;
			CurrentHealth = FMath::Max(CurrentHealth - DotDmg, 0.0f);
			if (CurrentHealth <= 0.0f && !bIsDead)
			{
				bIsDead = true;
				OnDeath();
				return;
			}
		}

		if (Effect.RemainingDuration <= 0.0f)
		{
			ActiveEffects.RemoveAt(i);
		}
	}
}

float ATDIEnemyBase::ComputeSpeedMultiplier() const
{
	float Multiplier = 1.0f;
	for (const FTDIStatusEffect& Effect : ActiveEffects)
	{
		Multiplier *= Effect.SlowMultiplier;
	}
	return FMath::Max(Multiplier, 0.1f);
}
