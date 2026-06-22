#include "TDIHUDBase.h"
#include "TDIGameMode.h"
#include "Subsystems/TDIResourceSubsystem.h"
#include "Research/TDIResearchManager.h"
#include "Waves/TDIWaveManager.h"
#include "Kismet/GameplayStatics.h"

void UTDIHUDBase::NativeConstruct()
{
	Super::NativeConstruct();

	GameMode = Cast<ATDIGameMode>(UGameplayStatics::GetGameMode(this));
	ResourceSubsystem = GetWorld()->GetSubsystem<UTDIResourceSubsystem>();
	WaveManager = Cast<ATDIWaveManager>(
		UGameplayStatics::GetActorOfClass(this, ATDIWaveManager::StaticClass()));

	// Bind resource delegate
	if (ResourceSubsystem)
	{
		ResourceSubsystem->OnResourceChanged.AddDynamic(this, &UTDIHUDBase::HandleResourceChanged);
	}

	// Bind wave delegates
	if (WaveManager)
	{
		WaveManager->OnWaveStarted.AddDynamic(this, &UTDIHUDBase::HandleWaveStarted);
		WaveManager->OnPreparationTimerUpdated.AddDynamic(this, &UTDIHUDBase::HandlePreparationTimer);
	}

	// Bind research delegate
	if (ATDIResearchManager* RM = Cast<ATDIResearchManager>(
		UGameplayStatics::GetActorOfClass(this, ATDIResearchManager::StaticClass())))
	{
		RM->OnResearchUnlocked.AddDynamic(this, &UTDIHUDBase::HandleResearchUnlocked);
	}
}

void UTDIHUDBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

float UTDIHUDBase::GetResourceAmount(FGameplayTag ResourceType) const
{
	return ResourceSubsystem ? ResourceSubsystem->GetResource(ResourceType) : 0.0f;
}

int32 UTDIHUDBase::GetCurrentWave() const
{
	return WaveManager ? WaveManager->GetCurrentWaveNumber() : 0;
}

EGamePhase UTDIHUDBase::GetCurrentPhase() const
{
	return GameMode ? GameMode->GetCurrentPhase() : EGamePhase::Preparation;
}

void UTDIHUDBase::HandleResourceChanged(FGameplayTag ResourceType, float NewAmount)
{
	OnResourceChanged(ResourceType, NewAmount);
}

void UTDIHUDBase::HandleWaveStarted(int32 WaveNumber)
{
	const int32 Total = WaveManager ? WaveManager->GetTotalWaves() : 0;
	OnWaveStarted(WaveNumber, Total);
}

void UTDIHUDBase::HandlePreparationTimer(float Remaining, float Total)
{
	OnPreparationTimerUpdated(Remaining, Total);
}

void UTDIHUDBase::HandleResearchUnlocked(FGameplayTag ResearchTag)
{
	OnResearchUnlocked(ResearchTag);
}
