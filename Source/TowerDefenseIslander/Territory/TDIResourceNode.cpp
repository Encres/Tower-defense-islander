#include "TDIResourceNode.h"
#include "TDIOutpost.h"
#include "PaperSpriteComponent.h"
#include "TimerManager.h"

ATDIResourceNode::ATDIResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SetRootComponent(SpriteComponent);
}

void ATDIResourceNode::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(ProductionTimer, this, &ATDIResourceNode::ProduceTick,
		ProductionInterval, true);
}

void ATDIResourceNode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ProductionTimer);
	Super::EndPlay(EndPlayReason);
}

void ATDIResourceNode::SetOutpost(ATDIOutpost* InOutpost)
{
	AssignedOutpost = InOutpost;
}

void ATDIResourceNode::SetProductionMultiplier(float Multiplier)
{
	ProductionMultiplier = FMath::Max(Multiplier, 0.0f);
}

void ATDIResourceNode::ProduceTick()
{
	if (!AssignedOutpost || !AssignedOutpost->IsActive()) return;
	if (!ResourceType.IsValid()) return;

	const float Amount = ProductionPerTick * ProductionMultiplier;
	AssignedOutpost->DepositResource(ResourceType, Amount);
}
