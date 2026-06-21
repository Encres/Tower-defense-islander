#include "TDITopDownPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

ATDITopDownPawn::ATDITopDownPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root scene component to anchor the camera rig
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	// Point straight down for a 2D top-down orthographic-style view
	SpringArm->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	SpringArm->TargetArmLength = TargetArmLength;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	// Orthographic projection keeps the visual style consistent as the camera zooms
	TopDownCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	TopDownCamera->OrthoWidth = 2048.0f;

	PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("PawnMovement"));
	PawnMovement->MaxSpeed = PanSpeed;
	PawnMovement->Acceleration = PanSpeed * 4.0f;
	PawnMovement->Deceleration = PanSpeed * 8.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void ATDITopDownPawn::BeginPlay()
{
	Super::BeginPlay();
	TargetArmLength = SpringArm->TargetArmLength;
}

void ATDITopDownPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smoothly interpolate arm length for zoom feel
	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength, TargetArmLength, DeltaTime, 8.0f);

	// Mirror orthographic width so zoom always matches what the player expects
	TopDownCamera->OrthoWidth = SpringArm->TargetArmLength * 1.7f;
}

void ATDITopDownPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ATDITopDownPawn::ZoomCamera(float Delta)
{
	TargetArmLength = FMath::Clamp(TargetArmLength - Delta * ZoomSpeed, MinArmLength, MaxArmLength);
}
