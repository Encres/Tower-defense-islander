#include "TDISaveGame.h"

const FString UTDISaveGame::SaveSlotPrefix = TEXT("TDI_Save_Slot");

FString UTDISaveGame::GetSlotName(int32 SlotIndex)
{
	return FString::Printf(TEXT("%s%d"), *SaveSlotPrefix, SlotIndex);
}
