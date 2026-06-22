#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TDIDataTypes.h"
#include "TDIResourceNode.generated.h"

class UPaperSpriteComponent;
class ATDIOutpost;

// Resource production node placed inside a territory.
// Produces resources at a configurable tick rate and deposits them into the
// local outpost storage. Production pauses if the outpost is inactive.
UCLASS()
class TOWERDEFENSEISLANDER_API ATDIResourceNode : public AActor
{
	GENERATED_BODY()

public:
	ATDIResourceNode();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetOutpost(ATDIOutpost* InOutpost);

	UFUNCTION(BlueprintPure, Category = "ResourceNode")
	float GetProductionRate() const { return ProductionPerTick; }

	UFUNCTION(BlueprintCallable, Category = "ResourceNode")
	void SetProductionMultiplier(float Multiplier);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	// Which resource this node produces
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ResourceNode")
	FGameplayTag ResourceType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ResourceNode", meta = (ClampMin = "0.1"))
	float ProductionPerTick = 10.0f;

	// Seconds between production ticks
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ResourceNode", meta = (ClampMin = "1"))
	float ProductionInterval = 5.0f;

private:
	UPROPERTY()
	TObjectPtr<ATDIOutpost> AssignedOutpost;

	float ProductionMultiplier = 1.0f;
	FTimerHandle ProductionTimer;

	UFUNCTION()
	void ProduceTick();
};
