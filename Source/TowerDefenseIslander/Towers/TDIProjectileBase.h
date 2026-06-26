#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TDIDataTypes.h"
#include "TDIProjectileBase.generated.h"

class UPaperSpriteComponent;
class UProjectileMovementComponent;
class ATDIEnemyBase;

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ATDIProjectileBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Called by the firing tower to set target, damage, and AoE radius
	void Initialize(ATDIEnemyBase* InTarget, float InDamage, float InSplashRadius, AActor* InInstigator);

	// Set status effect to apply on hit
	void ApplyEffect(float SlowAmount, float DotDamage, float DotDuration);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "100"))
	float Speed = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "0"))
	float MaxLifetime = 5.0f;

	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
	void OnHit(ATDIEnemyBase* HitEnemy);
	virtual void OnHit_Implementation(ATDIEnemyBase* HitEnemy);

private:
	UPROPERTY()
	TWeakObjectPtr<ATDIEnemyBase> Target;

	UPROPERTY()
	TObjectPtr<AActor> TowerInstigator;

	float Damage = 0.0f;
	float SplashRadius = 0.0f;
	float PendingSlowAmount = 0.0f;
	float PendingDotDamage = 0.0f;
	float PendingDotDuration = 0.0f;
	float LifeTimer = 0.0f;

	void DetonateAtLocation(const FVector& Location);
};
