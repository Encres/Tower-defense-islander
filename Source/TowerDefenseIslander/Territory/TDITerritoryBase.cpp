#include "TDITerritoryBase.h"
#include "TDIOutpost.h"
#include "TDIMonsterSpawner.h"
#include "Data/TDITerritoryData.h"
#include "Subsystems/TDITerritorySubsystem.h"
#include "Subsystems/TDIResourceSubsystem.h"
#include "Subsystems/TDILogisticsSubsystem.h"
#include "Research/TDIResearchManager.h"
#include "PaperSpriteComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

ATDITerritoryBase::ATDITerritoryBase()
{
	PrimaryActorTick.bCanEverTick = true;

	TerritoryBounds = CreateDefaultSubobject<UCapsuleComponent>(TEXT("TerritoryBounds"));
	SetRootComponent(TerritoryBounds);
	TerritoryBounds->SetCapsuleSize(512.0f, 64.0f);
	TerritoryBounds->SetCollisionProfileName(TEXT("NoCollision"));

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(RootComponent);
}

void ATDITerritoryBase::BeginPlay()
{
	Super::BeginPlay();

	if (UTDITerritorySubsystem* Sub = GetWorld()->GetSubsystem<UTDITerritorySubsystem>())
	{
		Sub->RegisterTerritory(this);
	}

	// Cache all spawners that are children of this territory
	TArray<AActor*> Attached;
	GetAttachedActors(Attached);
	for (AActor* A : Attached)
	{
		if (ATDIMonsterSpawner* Spawner = Cast<ATDIMonsterSpawner>(A))
		{
			Spawners.Add(Spawner);
		}
	}
}

void ATDITerritoryBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bCapturing || !TerritoryData) return;

	CaptureProgress += DeltaTime / TerritoryData->CaptureTime;
	CaptureProgress = FMath::Clamp(CaptureProgress, 0.0f, 1.0f);
	OnCaptureProgressUpdated.Broadcast(CaptureProgress);

	if (CaptureProgress >= 1.0f)
	{
		bCapturing = false;
		ChangeState(ETerritoryState::Captured);
		OnCaptureCompleted();
	}
}

void ATDITerritoryBase::BeginCapture()
{
	if (TerritoryState != ETerritoryState::Wild) return;
	bCapturing = true;
	ChangeState(ETerritoryState::Contested);
}

void ATDITerritoryBase::InterruptCapture(float ProgressSetback)
{
	if (TerritoryState != ETerritoryState::Contested) return;
	CaptureProgress = FMath::Max(CaptureProgress - ProgressSetback, 0.0f);
}

bool ATDITerritoryBase::BuildOutpost()
{
	if (TerritoryState != ETerritoryState::Captured) return false;
	if (Outpost) return false;
	if (!OutpostClass) return false;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Outpost = GetWorld()->SpawnActor<ATDIOutpost>(OutpostClass, GetActorLocation(), FRotator::ZeroRotator, Params);

	if (Outpost)
	{
		Outpost->Initialize(this);
		// Trigger logistics re-evaluation
		if (UTDILogisticsSubsystem* Logistics = GetWorld()->GetSubsystem<UTDILogisticsSubsystem>())
		{
			Logistics->ValidateNetwork();
		}
		return true;
	}
	return false;
}

void ATDITerritoryBase::RestoreState(ETerritoryState NewState, float Progress)
{
	CaptureProgress = Progress;
	TerritoryState = NewState;

	if (NewState == ETerritoryState::Captured)
	{
		bCapturing = false;
		OnCaptureCompleted();
	}
	else if (NewState == ETerritoryState::Contested)
	{
		bCapturing = true;
	}
}

void ATDITerritoryBase::OnCaptureCompleted_Implementation()
{
	// Unlock research for this territory type
	if (TerritoryData && TerritoryData->ResearchUnlockedOnCapture.IsValid())
	{
		if (ATDIResearchManager* RM = Cast<ATDIResearchManager>(
			UGameplayStatics::GetActorOfClass(this, ATDIResearchManager::StaticClass())))
		{
			RM->UnlockByTag(TerritoryData->ResearchUnlockedOnCapture);
		}
	}

	// Build outpost automatically
	BuildOutpost();

	// Silence territory spawners
	for (TObjectPtr<ATDIMonsterSpawner> Spawner : Spawners)
	{
		if (Spawner) Spawner->SetCapture(true);
	}
}

void ATDITerritoryBase::OnTerritoryLost_Implementation()
{
	if (Outpost)
	{
		Outpost->SetActive(false);
	}

	// Re-activate spawners
	for (TObjectPtr<ATDIMonsterSpawner> Spawner : Spawners)
	{
		if (Spawner) Spawner->SetCapture(false);
	}

	ChangeState(ETerritoryState::Wild);
	CaptureProgress = 0.0f;
}

void ATDITerritoryBase::ChangeState(ETerritoryState NewState)
{
	TerritoryState = NewState;
	OnStateChanged.Broadcast(this, NewState);

	if (UTDITerritorySubsystem* Sub = GetWorld()->GetSubsystem<UTDITerritorySubsystem>())
	{
		Sub->NotifyTerritoryStatChanged(this, NewState);
	}
}
