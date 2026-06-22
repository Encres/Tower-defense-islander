#include "TDIEnemyAIController.h"
#include "TDIEnemyBase.h"
#include "Logistics/TDIRoad.h"
#include "Territory/TDIOutpost.h"
#include "Towers/TDITowerBase.h"
#include "Castle/TDICastle.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"

ATDIEnemyAIController::ATDIEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ATDIEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledEnemy = Cast<ATDIEnemyBase>(InPawn);

	if (ControlledEnemy)
	{
		// Inherit objective from enemy data
		CurrentObjective = ControlledEnemy->GetPrimaryObjective();
	}

	if (BehaviorTree)
	{
		BlackboardComp->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
		BehaviorTreeComp->StartTree(*BehaviorTree);

		if (ControlledEnemy)
		{
			BlackboardComp->SetValueAsVector(TDIBlackboardKeys::PatrolOrigin,
				ControlledEnemy->GetActorLocation());
		}
	}

	// Perform initial target search
	RefreshTarget();
}

void ATDIEnemyAIController::OnUnPossess()
{
	BehaviorTreeComp->StopTree(EBTStopMode::Safe);
	Super::OnUnPossess();
}

void ATDIEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TargetRefreshTimer -= DeltaTime;
	if (TargetRefreshTimer <= 0.0f)
	{
		RefreshTarget();
		TargetRefreshTimer = TargetRefreshInterval;
	}

	// Update path progress for tower targeting priority
	if (ControlledEnemy && BlackboardComp)
	{
		if (AActor* Target = Cast<AActor>(
			BlackboardComp->GetValueAsObject(TDIBlackboardKeys::TargetActor)))
		{
			if (ATDICastle* Castle = Cast<ATDICastle>(Target))
			{
				// Approximate path progress by distance to castle
				const float MaxDist = 5000.0f;
				const float Dist = FVector::Dist(ControlledEnemy->GetActorLocation(),
					Castle->GetActorLocation());
				ControlledEnemy->SetPathProgress(FMath::Clamp(1.0f - Dist / MaxDist, 0.0f, 1.0f));
			}
		}
	}
}

void ATDIEnemyAIController::SetPrimaryTarget(AActor* Target)
{
	if (!BlackboardComp || !Target) return;
	BlackboardComp->SetValueAsObject(TDIBlackboardKeys::TargetActor, Target);
	BlackboardComp->SetValueAsVector(TDIBlackboardKeys::TargetLocation, Target->GetActorLocation());
}

void ATDIEnemyAIController::SetObjective(EEnemyObjective Objective)
{
	CurrentObjective = Objective;
	BlackboardComp->SetValueAsEnum(TDIBlackboardKeys::CurrentObjective,
		static_cast<uint8>(Objective));
	RefreshTarget();
}

void ATDIEnemyAIController::RefreshTarget()
{
	AActor* BestTarget = nullptr;

	// Prioritize objectives: Road > Outpost > Tower > Castle
	switch (CurrentObjective)
	{
	case EEnemyObjective::AttackRoad:
		BestTarget = FindBestRoadTarget();
		if (!BestTarget) BestTarget = FindBestOutpostTarget();
		break;
	case EEnemyObjective::AttackOutpost:
		BestTarget = FindBestOutpostTarget();
		if (!BestTarget) BestTarget = FindBestRoadTarget();
		break;
	case EEnemyObjective::AttackTower:
		BestTarget = FindBestTowerTarget();
		if (!BestTarget) BestTarget = FindBestOutpostTarget();
		break;
	case EEnemyObjective::AttackCastle:
	default:
		BestTarget = FindCastle();
		break;
	}

	// Final fallback: always march toward the castle
	if (!BestTarget) BestTarget = FindCastle();

	if (BestTarget) SetPrimaryTarget(BestTarget);
}

AActor* ATDIEnemyAIController::FindBestRoadTarget() const
{
	if (!ControlledEnemy) return nullptr;

	TArray<AActor*> RoadActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATDIRoad::StaticClass(), RoadActors);

	ATDIRoad* BestRoad = nullptr;
	float BestDist = FLT_MAX;

	for (AActor* A : RoadActors)
	{
		ATDIRoad* Road = Cast<ATDIRoad>(A);
		if (!Road || Road->GetRoadState() == ERoadState::Destroyed) continue;

		const float Dist = FVector::Dist(ControlledEnemy->GetActorLocation(), Road->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestRoad = Road;
		}
	}
	return BestRoad;
}

AActor* ATDIEnemyAIController::FindBestOutpostTarget() const
{
	if (!ControlledEnemy) return nullptr;

	TArray<AActor*> OutpostActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATDIOutpost::StaticClass(), OutpostActors);

	ATDIOutpost* Nearest = nullptr;
	float BestDist = FLT_MAX;

	for (AActor* A : OutpostActors)
	{
		ATDIOutpost* Outpost = Cast<ATDIOutpost>(A);
		if (!Outpost || !Outpost->IsActive()) continue;

		const float Dist = FVector::Dist(ControlledEnemy->GetActorLocation(), Outpost->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			Nearest = Outpost;
		}
	}
	return Nearest;
}

AActor* ATDIEnemyAIController::FindBestTowerTarget() const
{
	if (!ControlledEnemy) return nullptr;

	TArray<AActor*> TowerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATDITowerBase::StaticClass(), TowerActors);

	ATDITowerBase* Nearest = nullptr;
	float BestDist = FLT_MAX;

	for (AActor* A : TowerActors)
	{
		ATDITowerBase* Tower = Cast<ATDITowerBase>(A);
		if (!Tower) continue;

		const float Dist = FVector::Dist(ControlledEnemy->GetActorLocation(), Tower->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			Nearest = Tower;
		}
	}
	return Nearest;
}

AActor* ATDIEnemyAIController::FindCastle() const
{
	return UGameplayStatics::GetActorOfClass(GetWorld(), ATDICastle::StaticClass());
}
