#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDIGameMode.h"
#include "TDIHUDWidget.generated.h"

UCLASS()
class TOWERDEFENSEISLANDER_API UTDIHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnGoldUpdated(int32 NewGold);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnWaveStarted(int32 WaveNumber);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnPhaseChanged(EGamePhase NewPhase);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	class ATDIGameMode* GameMode;
};
