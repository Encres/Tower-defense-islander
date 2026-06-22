#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TDIDataTypes.h"
#include "TDIWorkerCart.generated.h"

class UPaperSpriteComponent;
class ATDIRoad;
class ATDIOutpost;
class ATDICastle;

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIWorkerCart : public AActor
{
	GENERATED_BODY()

public:
	ATDIWorkerCart();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	void Initialize(ATDIRoad* Road, ATDIOutpost* Outpost, ATDICastle* Castle);

	UFUNCTION(BlueprintPure, Category = "Cart")
	ECartState GetCartState() const { return CartState; }

	UFUNCTION(BlueprintPure, Category = "Cart")
	float GetLoadPercent() const;

	// Called by logistics subsystem or research upgrades
	UFUNCTION(BlueprintCallable, Category = "Cart")
	void SetCartCapacity(float NewCapacity);

	UFUNCTION(BlueprintCallable, Category = "Cart")
	void SetTravelSpeedMultiplier(float Multiplier);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cart", meta = (ClampMin = "10"))
	float CartCapacity = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cart", meta = (ClampMin = "100"))
	float BaseTravelSpeed = 300.0f;

	// Seconds spent loading/unloading
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cart", meta = (ClampMin = "0.5"))
	float LoadUnloadTime = 2.0f;

private:
	ECartState CartState = ECartState::Idle;

	UPROPERTY()
	TObjectPtr<ATDIRoad> AssignedRoad;

	UPROPERTY()
	TObjectPtr<ATDIOutpost> AssignedOutpost;

	UPROPERTY()
	TObjectPtr<ATDICastle> DestinationCastle;

	TArray<FTDIResourceAmount> Cargo;
	float TravelSpeedMultiplier = 1.0f;
	float SplineProgress = 0.0f;   // 0 = outpost end, 1 = castle end
	bool bTravelingToCastle = false;
	float ActionTimer = 0.0f;

	void SetState(ECartState NewState);
	void UpdateMovement(float DeltaTime);
	void StartLoading();
	void StartUnloading();
	void OnLoadComplete();
	void OnUnloadComplete();
};
