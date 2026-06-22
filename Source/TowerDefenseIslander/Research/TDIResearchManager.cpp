#include "TDIResearchManager.h"
#include "Data/TDIResearchData.h"
#include "Subsystems/TDIResourceSubsystem.h"

ATDIResearchManager::ATDIResearchManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATDIResearchManager::BeginPlay()
{
	Super::BeginPlay();
}

void ATDIResearchManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CurrentResearch) return;

	ResearchTimer -= DeltaTime;
	if (ResearchTimer <= 0.0f)
	{
		// Research complete
		const FGameplayTag Tag = CurrentResearch->ResearchTag;
		UnlockedResearch.Add(Tag);
		OnResearchUnlocked.Broadcast(Tag);

		CurrentResearch = nullptr;
		ResearchTimer = 0.0f;
		RefundCache.Empty();
	}
}

bool ATDIResearchManager::IsUnlocked(FGameplayTag ResearchTag) const
{
	return UnlockedResearch.Contains(ResearchTag);
}

bool ATDIResearchManager::CanUnlock(UTDIResearchData* Research) const
{
	if (!Research) return false;
	if (IsUnlocked(Research->ResearchTag)) return false;
	if (IsResearching()) return false;

	// Check all prerequisites
	for (const FGameplayTag& Prereq : Research->Prerequisites)
	{
		if (!IsUnlocked(Prereq)) return false;
	}

	// Check resource cost affordability
	if (UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>())
	{
		return Resources->HasResources(Research->Cost);
	}
	return false;
}

float ATDIResearchManager::GetResearchProgress() const
{
	if (!CurrentResearch || CurrentResearch->ResearchTime <= 0.0f) return 0.0f;
	return 1.0f - (ResearchTimer / CurrentResearch->ResearchTime);
}

TArray<UTDIResearchData*> ATDIResearchManager::GetAvailableResearch() const
{
	TArray<UTDIResearchData*> Available;
	for (TObjectPtr<UTDIResearchData> Node : AllResearchNodes)
	{
		if (Node && !IsUnlocked(Node->ResearchTag))
		{
			// Check prerequisites are met
			bool bPrereqMet = true;
			for (const FGameplayTag& Prereq : Node->Prerequisites)
			{
				if (!IsUnlocked(Prereq)) { bPrereqMet = false; break; }
			}
			if (bPrereqMet) Available.Add(Node);
		}
	}
	return Available;
}

bool ATDIResearchManager::StartResearch(UTDIResearchData* Research)
{
	if (!CanUnlock(Research)) return false;

	UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>();
	if (!Resources) return false;
	if (!Resources->SpendResources(Research->Cost)) return false;

	CurrentResearch = Research;
	ResearchTimer = Research->ResearchTime;
	RefundCache = Research->Cost;

	OnResearchStarted.Broadcast(Research);
	return true;
}

void ATDIResearchManager::CancelResearch()
{
	if (!CurrentResearch) return;

	// Refund the resource cost
	if (UTDIResourceSubsystem* Resources = GetWorld()->GetSubsystem<UTDIResourceSubsystem>())
	{
		for (const FTDIResourceAmount& Refund : RefundCache)
		{
			Resources->AddResource(Refund.ResourceType, Refund.Amount);
		}
	}

	CurrentResearch = nullptr;
	ResearchTimer = 0.0f;
	RefundCache.Empty();
}

void ATDIResearchManager::UnlockByTag(FGameplayTag ResearchTag)
{
	if (!ResearchTag.IsValid()) return;
	UnlockedResearch.Add(ResearchTag);
	OnResearchUnlocked.Broadcast(ResearchTag);
}

TArray<FString> ATDIResearchManager::SerializeUnlocks() const
{
	TArray<FString> Result;
	for (const FGameplayTag& Tag : UnlockedResearch)
	{
		Result.Add(Tag.ToString());
	}
	return Result;
}

void ATDIResearchManager::DeserializeUnlocks(const TArray<FString>& TagStrings)
{
	UnlockedResearch.Empty();
	for (const FString& TagStr : TagStrings)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagStr));
		if (Tag.IsValid()) UnlockedResearch.Add(Tag);
	}
}
