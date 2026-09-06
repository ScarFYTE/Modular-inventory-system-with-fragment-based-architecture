#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryUIStyle.h"   // for UInventoryUIStyle and EInventoryStylePreset
#include "InventoryUIConfig.generated.h"



UENUM(BlueprintType)
enum class EInventoryStyleMode : uint8
{
	Preset   UMETA(DisplayName = "Use Preset"),
	Custom   UMETA(DisplayName = "Use Custom Style")
};


UCLASS(BlueprintType)
class LEARNINGC_API UInventoryUIConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layout")
	int32 ColumnsPerRow = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layout")
	FVector2D SlotSize = FVector2D(80.f, 80.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layout")
	FVector2D SlotPadding = FVector2D(4.f, 4.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screen Placement")
	FVector2D ScreenPosition = FVector2D(100.f, 100.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Style")
	EInventoryStyleMode StyleMode = EInventoryStyleMode::Preset;

	// Used when StyleMode == Preset
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Style", meta = (EditCondition = "StyleMode == EInventoryStyleMode::Preset", EditConditionHides))
	EInventoryStylePreset Preset = EInventoryStylePreset::Minimalist;

	// Used when StyleMode == Custom
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Style", meta = (EditCondition = "StyleMode == EInventoryStyleMode::Custom", EditConditionHides))
	UInventoryUIStyle* CustomStyle = nullptr;

	// Assign your minimalist/fantasy/etc preset assets here so presets can resolve to them
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Style|Preset Library")
	TMap<EInventoryStylePreset, UInventoryUIStyle*> PresetLibrary;

	// Resolves the active style based on StyleMode/Preset/CustomStyle
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Style")
	UInventoryUIStyle* GetResolvedStyle() const;
};
