#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TDITopDownPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;

// Camera pawn that provides the 2D top-down view. No visible mesh — purely
// a camera rig the PlayerController uses to pan and zoom over the island.
UCLASS()
class TOWERDEFENSEISLANDER_API ATDITopDownPawn : public APawn
{
	GENERATED_BODY()

public:
	ATDITopDownPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ZoomCamera(float Delta);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* TopDownCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UFloatingPawnMovement* PawnMovement;

	// Camera zoom limits in world units (spring arm length)
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float MinArmLength = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float MaxArmLength = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float ZoomSpeed = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float PanSpeed = 800.0f;

private:
	float TargetArmLength = 1200.0f;
};
