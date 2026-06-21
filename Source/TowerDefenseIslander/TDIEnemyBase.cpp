#include "TDIEnemyBase.h"
#include "TDIGameMode.h"
#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ATDIEnemyBase::ATDIEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;
		Move->MaxWalkSpeed = MoveSpeed;
		// Flat movement for top-down 2D — no gravity influence on gameplay plane
		Move->GravityScale = 0.0f;
		Move->MovementMode = MOVE_Flying;
	}

	bUseControllerRotationYaw = false;
}

void ATDIEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void ATDIEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MoveAlongPath(DeltaTime);
}

void ATDIEnemyBase::InitializePath(USplineComponent* PathSpline)
{
	AssignedPath = PathSpline;
	DistanceTravelled = 0.0f;
	PathProgress = 0.0f;

	if (AssignedPath)
	{
		SetActorLocation(AssignedPath->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World));
	}
}

void ATDIEnemyBase::MoveAlongPath(float DeltaTime)
{
	if (!AssignedPath) return;

	const float TotalLength = AssignedPath->GetSplineLength();
	DistanceTravelled += MoveSpeed * DeltaTime;
	PathProgress = FMath::Clamp(DistanceTravelled / TotalLength, 0.0f, 1.0f);

	if (DistanceTravelled >= TotalLength)
	{
		ReachedBase();
		return;
	}

	FVector NewLocation = AssignedPath->GetLocationAtDistanceAlongSpline(DistanceTravelled, ESplineCoordinateSpace::World);
	NewLocation.Z = GetActorLocation().Z;
	SetActorLocation(NewLocation);

	FVector Direction = AssignedPath->GetDirectionAtDistanceAlongSpline(DistanceTravelled, ESplineCoordinateSpace::World);
	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}
}

void ATDIEnemyBase::ReachedBase()
{
	if (ATDIGameMode* GM = Cast<ATDIGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->OnBaseDestroyed();
	}
	Destroy();
}

float ATDIEnemyBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth -= Applied;
	if (CurrentHealth <= 0.0f)
	{
		OnDeath();
	}
	return Applied;
}

void ATDIEnemyBase::OnDeath_Implementation()
{
	if (ATDIGameMode* GM = Cast<ATDIGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->AddGold(GoldReward);
		GM->NotifyEnemyKilled();
	}
	Destroy();
}
