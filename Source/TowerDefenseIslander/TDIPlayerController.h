#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TDIPlayerController.generated.h"

class ATDITowerBase;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class TOWERDEFENSEISLANDER_API ATDIPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATDIPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Tower Placement")
	void SelectTowerForPlacement(TSubclassOf<ATDITowerBase> TowerClass);

	UFUNCTION(BlueprintCallable, Category = "Tower Placement")
	void CancelPlacement();

	UFUNCTION(BlueprintPure, Category = "Tower Placement")
	bool IsPlacingTower() const { return bIsPlacingTower; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_PlaceTower;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_CancelAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_ScrollCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_PanCamera;

private:
	bool bIsPlacingTower = false;
	TSubclassOf<ATDITowerBase> PendingTowerClass;
	ATDITowerBase* GhostTower = nullptr;

	void OnPlaceTowerInput(const FInputActionValue& Value);
	void OnCancelInput(const FInputActionValue& Value);
	void OnScrollCameraInput(const FInputActionValue& Value);
	void OnPanCameraInput(const FInputActionValue& Value);

	bool GetGroundPositionUnderCursor(FVector& OutPosition) const;
	void UpdateGhostTowerPosition();
};
