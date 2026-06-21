#include "TDIWaveSpawner.h"
#include "TDIEnemyBase.h"
#include "Components/SplineComponent.h"

ATDIWaveSpawner::ATDIWaveSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	EnemyPath = CreateDefaultSubobject<USplineComponent>(TEXT("EnemyPath"));
	SetRootComponent(EnemyPath);
}

void ATDIWaveSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void ATDIWaveSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bSpawning) return;

	SpawnTimer -= DeltaTime;
	if (SpawnTimer <= 0.0f)
	{
		SpawnNext();
	}
}

void ATDIWaveSpawner::StartWave(int32 WaveIndex)
{
	if (!WaveDefinitions.IsValidIndex(WaveIndex)) return;

	ActiveWaveIndex = WaveIndex;
	CurrentGroup = 0;
	SpawnedInGroup = 0;
	bSpawning = true;
	SpawnTimer = 0.0f;
}

void ATDIWaveSpawner::SpawnNext()
{
	if (!WaveDefinitions.IsValidIndex(ActiveWaveIndex)) return;

	const FWaveDefinition& Wave = WaveDefinitions[ActiveWaveIndex];
	if (!Wave.Groups.IsValidIndex(CurrentGroup))
	{
		bSpawning = false;
		return;
	}

	const FEnemySpawnEntry& Group = Wave.Groups[CurrentGroup];
	if (!Group.EnemyClass) return;

	FVector SpawnLoc = EnemyPath->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ATDIEnemyBase* Enemy = GetWorld()->SpawnActor<ATDIEnemyBase>(Group.EnemyClass, SpawnLoc, FRotator::ZeroRotator, Params);
	if (Enemy)
	{
		Enemy->InitializePath(EnemyPath);
	}

	SpawnedInGroup++;
	SpawnTimer = Group.SpawnInterval;

	if (SpawnedInGroup >= Group.Count)
	{
		CurrentGroup++;
		SpawnedInGroup = 0;
	}
}
