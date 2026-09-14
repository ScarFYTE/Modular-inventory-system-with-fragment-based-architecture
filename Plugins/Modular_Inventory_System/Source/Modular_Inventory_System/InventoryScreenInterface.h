#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryScreenInterface.generated.h"

class UInventoryComponent;

UINTERFACE(BlueprintType)
class MODULAR_INVENTORY_SYSTEM_API UInventoryScreenInterface : public UInterface
{
	GENERATED_BODY()
};

class MODULAR_INVENTORY_SYSTEM_API IInventoryScreenInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory|UI")
	void InitializeScreen(UInventoryComponent* TargetInventory);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory|UI")
	void RefreshScreen();
};