#include "InventoryUIConfig.h"

UInventoryUIStyle* UInventoryUIConfig::GetResolvedStyle() const
{
	if (StyleMode == EInventoryStyleMode::Custom)
	{
		return CustomStyle;
	}

	// Preset mode: look up the assigned asset for the chosen preset
	if (UInventoryUIStyle* const* Found = PresetLibrary.Find(Preset))
	{
		return *Found;
	}

	return nullptr;
}