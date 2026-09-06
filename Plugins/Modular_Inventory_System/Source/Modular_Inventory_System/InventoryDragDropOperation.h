#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryDragDropOperation.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MODULAR_INVENTORY_SYSTEM_API UInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 SourceSlotIndex = -1;
};