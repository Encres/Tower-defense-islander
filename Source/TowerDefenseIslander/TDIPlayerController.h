#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Data/TDIDataTypes.h"
#include "TDIPlayerController.generated.h"

class ATDITowerBase;
class ATDITerritoryBase;
class ATDIRoad;
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
	virtual void Tick(float DeltaTime) override;

	// ---- Tower Placement ----
	UFUNCTION(BlueprintCallable, Category = "Player|TowerPlacement")
	void SelectTowerForPlacement(TSubclassOf<ATDITowerBase> TowerClass);

	UFUNCTION(BlueprintCallable, Category = "Player|TowerPlacement")
	void CancelPlacement();

	UFUNCTION(BlueprintPure, Category = "Player|TowerPlacement")
	bool IsPlacingTower() const { return bIsPlacingTower; }

	// ---- Territory Capture ----
	UFUNCTION(BlueprintCallable, Category = "Player|Territory")
	bool AttemptCaptureTerritory(ATDITerritoryBase* Territory);

	// ---- Selection ----
	UFUNCTION(BlueprintPure, Category = "Player|Selection")
	AActor* GetSelectedActor() const { return SelectedActor; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_PrimaryAction;    // LMB: place/select

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_SecondaryAction;  // RMB: cancel/context

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_PanCamera;        // WASD / MMB drag

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ZoomCamera;       // Scroll wheel

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_QuickSave;        // F5

private:
	bool bIsPlacingTower = false;
	TSubclassOf<ATDITowerBase> PendingTowerClass;

	UPROPERTY()
	TObjectPtr<ATDITowerBase> GhostTower;

	UPROPERTY()
	TObjectPtr<AActor> SelectedActor;

	void OnPrimaryAction(const FInputActionValue& Value);
	void OnSecondaryAction(const FInputActionValue& Value);
	void OnPanCamera(const FInputActionValue& Value);
	void OnZoomCamera(const FInputActionValue& Value);
	void OnQuickSave(const FInputActionValue& Value);

	bool GetCursorWorldPosition(FVector& OutPos) const;
	void UpdateGhostTower();
	void HandleSelection();
	void NotifyHUDSelection(AActor* Actor);
};
