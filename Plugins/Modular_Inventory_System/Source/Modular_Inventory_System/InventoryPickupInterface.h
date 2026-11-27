#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryPickupInterface.generated.h"

class UItemDefinition;

UINTERFACE(BlueprintType, Blueprintable)
class MODULAR_INVENTORY_SYSTEM_API UInventoryPickupInterface : public UInterface
{
    GENERATED_BODY()
};

class MODULAR_INVENTORY_SYSTEM_API IInventoryPickupInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable,
        Category = "Inventory|Pickup")
    UItemDefinition* GetPickupItemDefinition() const;
};