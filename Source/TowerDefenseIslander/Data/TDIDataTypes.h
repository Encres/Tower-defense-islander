#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "TDIDataTypes.generated.h"

// ============================================================
// GAME PHASE
// ============================================================
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	MainMenu        UMETA(DisplayName = "Main Menu"),
	Preparation     UMETA(DisplayName = "Preparation"),
	Wave            UMETA(DisplayName = "Wave"),
	WaveComplete    UMETA(DisplayName = "Wave Complete"),
	GameOver        UMETA(DisplayName = "Game Over"),
	Victory         UMETA(DisplayName = "Victory")
};

// ============================================================
// TERRITORY
// ============================================================
UENUM(BlueprintType)
enum class ETerritoryState : uint8
{
	Wild        UMETA(DisplayName = "Wild"),
	Contested   UMETA(DisplayName = "Contested"),
	Captured    UMETA(DisplayName = "Captured")
};

UENUM(BlueprintType)
enum class ETerritoryType : uint8
{
	GoblinCamp          UMETA(DisplayName = "Goblin Camp"),
	SkeletonGraveyard   UMETA(DisplayName = "Skeleton Graveyard"),
	SpiderNest          UMETA(DisplayName = "Spider Nest"),
	OgreCamp            UMETA(DisplayName = "Ogre Camp"),
	DragonTerritory     UMETA(DisplayName = "Dragon Territory")
};

// ============================================================
// TOWERS
// ============================================================
UENUM(BlueprintType)
enum class ETowerType : uint8
{
	Arrow       UMETA(DisplayName = "Arrow Tower"),
	Cannon      UMETA(DisplayName = "Cannon Tower"),
	Frost       UMETA(DisplayName = "Frost Tower"),
	Fire        UMETA(DisplayName = "Fire Tower"),
	Lightning   UMETA(DisplayName = "Lightning Tower"),
	Poison      UMETA(DisplayName = "Poison Tower")
};

UENUM(BlueprintType)
enum class ETowerTier : uint8
{
	Tier1 = 0   UMETA(DisplayName = "Tier 1"),
	Tier2 = 1   UMETA(DisplayName = "Tier 2"),
	Tier3 = 2   UMETA(DisplayName = "Tier 3"),
	Tier4 = 3   UMETA(DisplayName = "Tier 4")
};

// ============================================================
// ENEMIES
// ============================================================
UENUM(BlueprintType)
enum class EEnemyObjective : uint8
{
	AttackRoad      UMETA(DisplayName = "Attack Road"),
	AttackOutpost   UMETA(DisplayName = "Attack Outpost"),
	AttackTower     UMETA(DisplayName = "Attack Tower"),
	AttackCastle    UMETA(DisplayName = "Attack Castle")
};

UENUM(BlueprintType)
enum class EEnemyMoveType : uint8
{
	Ground  UMETA(DisplayName = "Ground"),
	Flying  UMETA(DisplayName = "Flying"),
	Burrowing UMETA(DisplayName = "Burrowing")
};

// ============================================================
// LOGISTICS
// ============================================================
UENUM(BlueprintType)
enum class ECartState : uint8
{
	Idle                    UMETA(DisplayName = "Idle"),
	TravelingToTerritory    UMETA(DisplayName = "Traveling to Territory"),
	Loading                 UMETA(DisplayName = "Loading"),
	TravelingToCastle       UMETA(DisplayName = "Traveling to Castle"),
	Unloading               UMETA(DisplayName = "Unloading"),
	Disabled                UMETA(DisplayName = "Disabled (road broken)")
};

UENUM(BlueprintType)
enum class ERoadState : uint8
{
	Intact      UMETA(DisplayName = "Intact"),
	Damaged     UMETA(DisplayName = "Damaged"),
	Destroyed   UMETA(DisplayName = "Destroyed")
};

UENUM(BlueprintType)
enum class ERoadTier : uint8
{
	Dirt        UMETA(DisplayName = "Dirt"),
	Gravel      UMETA(DisplayName = "Gravel"),
	Stone       UMETA(DisplayName = "Stone"),
	Reinforced  UMETA(DisplayName = "Reinforced Stone")
};

// ============================================================
// RESOURCES
// ============================================================
USTRUCT(BlueprintType)
struct TOWERDEFENSEISLANDER_API FTDIResourceAmount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FGameplayTag ResourceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = "0"))
	float Amount = 0.0f;

	FTDIResourceAmount() = default;
	FTDIResourceAmount(FGameplayTag InType, float InAmount)
		: ResourceType(InType), Amount(InAmount) {}
};


// ============================================================
// TOWERS — per-tier stats block
// ============================================================
USTRUCT(BlueprintType)
struct TOWERDEFENSEISLANDER_API FTDITowerTierStats
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	float Damage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	float Range = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	float FireRate = 1.0f;      // attacks per second

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	float ProjectileSpeed = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	float SplashRadius = 0.0f;  // 0 = single target

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	float SlowAmount = 0.0f;    // 0–1; fraction of speed reduction

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	float DotDamagePerSec = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	float DotDuration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	TArray<FTDIResourceAmount> UpgradeCost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	FGameplayTag RequiredResearch;
};

// ============================================================
// STATUS EFFECTS
// ============================================================
USTRUCT(BlueprintType)
struct TOWERDEFENSEISLANDER_API FTDIStatusEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float SlowMultiplier = 1.0f;    // 1 = no slow, 0.5 = half speed

	UPROPERTY(BlueprintReadOnly)
	float DotDamagePerSec = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float RemainingDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EffectSource;      // which tower type applied this
};

// ============================================================
// SAVE — territory snapshot
// ============================================================
USTRUCT(BlueprintType)
struct TOWERDEFENSEISLANDER_API FTDITerritorySaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FString TerritoryID;

	UPROPERTY()
	ETerritoryState State = ETerritoryState::Wild;

	UPROPERTY()
	float CaptureProgress = 0.0f;
};

// ============================================================
// SAVE — tower snapshot
// ============================================================
USTRUCT(BlueprintType)
struct TOWERDEFENSEISLANDER_API FTDITowerSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FString TowerSlotID;

	UPROPERTY()
	ETowerType TowerType = ETowerType::Arrow;

	UPROPERTY()
	int32 Tier = 0;
};

// ============================================================
// SAVE — road snapshot
// ============================================================
USTRUCT(BlueprintType)
struct TOWERDEFENSEISLANDER_API FTDIRoadSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FString RoadID;

	UPROPERTY()
	ERoadTier Tier = ERoadTier::Dirt;

	UPROPERTY()
	float Health = 100.0f;
};

// ============================================================
// NATIVE GAMEPLAY TAGS — declared here, defined in TDIDataTypes.cpp
// ============================================================
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_Gold)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_Wood)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_Leather)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_Bone)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_DarkEssence)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_Silk)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_Venom)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_Stone)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_Iron)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_DragonScale)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Resource_Crystal)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Research_Tier1Towers)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Research_Tier2Towers)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Research_Tier3Towers)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Research_Tier4Towers)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Research_FasterCarts)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Research_ExtraWorkers)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Research_StrongerWalls)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Research_CartCapacity)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Effect_Slow)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Effect_Burn)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Effect_Poison)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TDITag_Effect_Stun)
