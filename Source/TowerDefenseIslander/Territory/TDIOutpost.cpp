#include "TDIOutpost.h"
#include "TDITerritoryBase.h"
#include "PaperSpriteComponent.h"
#include "Components/BoxComponent.h"

ATDIOutpost::ATDIOutpost()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetBoxExtent(FVector(48.0f, 48.0f, 24.0f));
	CollisionBox->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(RootComponent);
}

void ATDIOutpost::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void ATDIOutpost::Initialize(ATDITerritoryBase* InOwner)
{
	OwnerTerritory = InOwner;
	bIsActive = true;
}

float ATDIOutpost::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth = FMath::Max(CurrentHealth - Applied, 0.0f);

	if (CurrentHealth <= 0.0f && OwnerTerritory)
	{
		SetActive(false);
		OwnerTerritory->OnTerritoryLost();
	}

	return Applied;
}

void ATDIOutpost::SetActive(bool bActive)
{
	bIsActive = bActive;
}

void ATDIOutpost::DepositResource(FGameplayTag ResourceType, float Amount)
{
	if (!bIsActive || Amount <= 0.0f) return;

	float& Stored = LocalStorage.FindOrAdd(ResourceType, 0.0f);
	Stored = FMath::Min(Stored + Amount, LocalStorageCapacity);
}

TArray<FTDIResourceAmount> ATDIOutpost::PickupResources(float CartCapacity)
{
	TArray<FTDIResourceAmount> Cargo;
	if (!bIsActive) return Cargo;

	float RemainingCapacity = CartCapacity;

	for (auto& Pair : LocalStorage)
	{
		if (RemainingCapacity <= 0.0f) break;

		float TakeAmount = FMath::Min(Pair.Value, RemainingCapacity);
		if (TakeAmount > 0.0f)
		{
			Cargo.Add(FTDIResourceAmount(Pair.Key, TakeAmount));
			Pair.Value -= TakeAmount;
			RemainingCapacity -= TakeAmount;
		}
	}

	return Cargo;
}

float ATDIOutpost::GetStoredAmount(FGameplayTag ResourceType) const
{
	if (const float* Found = LocalStorage.Find(ResourceType))
	{
		return *Found;
	}
	return 0.0f;
}

float ATDIOutpost::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}
