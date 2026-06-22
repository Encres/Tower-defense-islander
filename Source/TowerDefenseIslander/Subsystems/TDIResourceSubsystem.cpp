#include "TDIResourceSubsystem.h"
#include "Data/TDIDataTypes.h"

void UTDIResourceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default storage capacities per resource type
	auto SetDefault = [this](FGameplayTag Tag, float Cap)
	{
		StorageCapacities.Add(Tag, Cap);
		ResourceLedger.Add(Tag, 0.0f);
	};

	SetDefault(TDITag_Resource_Gold,        9999.0f);   // Gold has no practical cap
	SetDefault(TDITag_Resource_Wood,         500.0f);
	SetDefault(TDITag_Resource_Leather,      500.0f);
	SetDefault(TDITag_Resource_Bone,         500.0f);
	SetDefault(TDITag_Resource_DarkEssence,  500.0f);
	SetDefault(TDITag_Resource_Silk,         500.0f);
	SetDefault(TDITag_Resource_Venom,        500.0f);
	SetDefault(TDITag_Resource_Stone,        500.0f);
	SetDefault(TDITag_Resource_Iron,         500.0f);
	SetDefault(TDITag_Resource_DragonScale,  500.0f);
	SetDefault(TDITag_Resource_Crystal,      500.0f);

	// Starting gold
	ResourceLedger[TDITag_Resource_Gold] = 150.0f;
}

float UTDIResourceSubsystem::GetResource(FGameplayTag ResourceType) const
{
	if (const float* Found = ResourceLedger.Find(ResourceType))
	{
		return *Found;
	}
	return 0.0f;
}

bool UTDIResourceSubsystem::HasResources(const TArray<FTDIResourceAmount>& Costs) const
{
	for (const FTDIResourceAmount& Cost : Costs)
	{
		if (GetResource(Cost.ResourceType) < Cost.Amount)
		{
			return false;
		}
	}
	return true;
}

float UTDIResourceSubsystem::GetStorageCapacity(FGameplayTag ResourceType) const
{
	return GetEffectiveCapacity(ResourceType);
}

void UTDIResourceSubsystem::AddResource(FGameplayTag ResourceType, float Amount)
{
	if (Amount <= 0.0f) return;

	float& Current = ResourceLedger.FindOrAdd(ResourceType, 0.0f);
	const float Cap = GetEffectiveCapacity(ResourceType);
	Current = FMath::Min(Current + Amount, Cap);

	OnResourceChanged.Broadcast(ResourceType, Current);
}

bool UTDIResourceSubsystem::SpendResources(const TArray<FTDIResourceAmount>& Costs)
{
	if (!HasResources(Costs)) return false;

	for (const FTDIResourceAmount& Cost : Costs)
	{
		DeductResource(Cost.ResourceType, Cost.Amount);
	}
	return true;
}

void UTDIResourceSubsystem::DeductResource(FGameplayTag ResourceType, float Amount)
{
	if (Amount <= 0.0f) return;

	float& Current = ResourceLedger.FindOrAdd(ResourceType, 0.0f);
	Current = FMath::Max(Current - Amount, 0.0f);
	OnResourceChanged.Broadcast(ResourceType, Current);
}

void UTDIResourceSubsystem::SetStorageCapacity(FGameplayTag ResourceType, float NewCapacity)
{
	StorageCapacities.Add(ResourceType, NewCapacity);
	OnStorageCapacityChanged.Broadcast(NewCapacity);

	// Clamp existing resources to new capacity
	if (float* Current = ResourceLedger.Find(ResourceType))
	{
		*Current = FMath::Min(*Current, GetEffectiveCapacity(ResourceType));
	}
}

void UTDIResourceSubsystem::SetGlobalCapacityMultiplier(float Multiplier)
{
	GlobalCapacityMultiplier = FMath::Max(Multiplier, 0.1f);
}

void UTDIResourceSubsystem::AddAllResources(float Amount)
{
	TArray<FGameplayTag> Tags;
	ResourceLedger.GetKeys(Tags);
	for (const FGameplayTag& Tag : Tags)
	{
		AddResource(Tag, Amount);
	}
}

TMap<FString, float> UTDIResourceSubsystem::SerializeToStringMap() const
{
	TMap<FString, float> Result;
	for (const auto& Pair : ResourceLedger)
	{
		Result.Add(Pair.Key.ToString(), Pair.Value);
	}
	return Result;
}

void UTDIResourceSubsystem::DeserializeFromStringMap(const TMap<FString, float>& Data)
{
	ResourceLedger.Empty();
	for (const auto& Pair : Data)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Pair.Key));
		if (Tag.IsValid())
		{
			ResourceLedger.Add(Tag, Pair.Value);
			OnResourceChanged.Broadcast(Tag, Pair.Value);
		}
	}
}

float UTDIResourceSubsystem::GetEffectiveCapacity(FGameplayTag ResourceType) const
{
	if (const float* Cap = StorageCapacities.Find(ResourceType))
	{
		return (*Cap) * GlobalCapacityMultiplier;
	}
	return 500.0f * GlobalCapacityMultiplier;
}
