#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "InventoryUIStyle.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlotStyle
{
	GENERATED_BODY()

	// Core, always-visible fields
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style")
	FSlateBrush FilledSlotBackground;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style")
	FSlateBrush EmptySlotBackground;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style")
	FSlateFontInfo CountFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style")
	FLinearColor CountTextColor = FLinearColor::White;

	// Advanced — tucked behind "Show Advanced" in the Details panel
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style", AdvancedDisplay)
	FSlateBrush HoveredSlotBackground;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style", AdvancedDisplay)
	FLinearColor SlotTint = FLinearColor::White;
};

UENUM(BlueprintType)
enum class EInventoryStylePreset : uint8
{
	Minimalist,
	Fantasy,
	SciFi,
	Dark
};

UCLASS(BlueprintType)
class MODULAR_INVENTORY_SYSTEM_API UInventoryUIStyle : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style")
	FInventorySlotStyle SlotStyle;
};