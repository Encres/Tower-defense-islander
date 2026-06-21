#include "TDIHUDWidget.h"
#include "TDIGameMode.h"
#include "Kismet/GameplayStatics.h"

void UTDIHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameMode = Cast<ATDIGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->OnGoldChanged.AddDynamic(this, &UTDIHUDWidget::OnGoldUpdated);
		GameMode->OnWaveStarted.AddDynamic(this, &UTDIHUDWidget::OnWaveStarted);
		GameMode->OnPhaseChanged.AddDynamic(this, &UTDIHUDWidget::OnPhaseChanged);
	}
}
