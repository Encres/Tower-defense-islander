#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDIEnemyBase.generated.h"

class UPaperSpriteComponent;
class USplineComponent;

UCLASS(Abstract)
class TOWERDEFENSEISLANDER_API ATDIEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	ATDIEnemyBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// 0–1 progress along the path (used by towers for target priority)
	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetPathProgress() const { return PathProgress; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	int32 GetGoldReward() const { return GoldReward; }

	// Called by the wave spawner to assign a path spline
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void InitializePath(USplineComponent* PathSpline);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats")
	float MoveSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats")
	int32 GoldReward = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats")
	int32 DamageToBase = 1;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy")
	void OnDeath();

	virtual void OnDeath_Implementation();

private:
	float CurrentHealth = 0.0f;
	float PathProgress = 0.0f;        // 0 = start, 1 = reached base
	float DistanceTravelled = 0.0f;

	UPROPERTY()
	USplineComponent* AssignedPath = nullptr;

	void MoveAlongPath(float DeltaTime);
	void ReachedBase();
};
