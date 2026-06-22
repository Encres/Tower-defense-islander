#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Data/TDIDataTypes.h"
#include "TDIEnemyAIController.generated.h"

class ATDIEnemyBase;
class ATDIRoad;
class ATDIOutpost;
class ATDITowerBase;
class ATDICastle;
class UBehaviorTree;
class UBehaviorTreeComponent;
class UBlackboardComponent;

// Blackboard key names
namespace TDIBlackboardKeys
{
	static const FName TargetActor      = TEXT("TargetActor");
	static const FName TargetLocation   = TEXT("TargetLocation");
	static const FName CurrentObjective = TEXT("CurrentObjective");
	static const FName bIsInCombat      = TEXT("bIsInCombat");
	static const FName PatrolOrigin     = TEXT("PatrolOrigin");
}

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	ATDIEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	// Called by the wave manager to assign the primary attack target
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetPrimaryTarget(AActor* Target);

	// Called by wave manager to make enemy prefer road targets
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetObjective(EEnemyObjective Objective);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBlackboardComponent> BlackboardComp;

private:
	UPROPERTY()
	TObjectPtr<ATDIEnemyBase> ControlledEnemy;

	EEnemyObjective CurrentObjective = EEnemyObjective::AttackCastle;

	// How often to re-evaluate the best target
	float TargetRefreshInterval = 3.0f;
	float TargetRefreshTimer = 0.0f;

	void RefreshTarget();

	AActor* FindBestRoadTarget() const;
	AActor* FindBestOutpostTarget() const;
	AActor* FindBestTowerTarget() const;
	AActor* FindCastle() const;
};
