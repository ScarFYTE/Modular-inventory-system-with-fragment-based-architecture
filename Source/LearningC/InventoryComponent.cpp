#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize a fixed-size array of empty slots
    InventoryItems.SetNum(SlotCount);
}

void UInventoryComponent::AddItem(UItemDefinition* Def, int32 Count)
{
    UE_LOG(LogTemp, Warning, TEXT("AddItem called. InventoryItems.Num() = %d, SlotCount = %d"), InventoryItems.Num(), SlotCount);
    if (!Def || Count <= 0)
    {
        return;
    }

    // 1. Try to stack onto an existing entry of the same item
    for (FInventoryItem& Item : InventoryItems)
    {
        if (Item.ItemDef == Def)
        {
            Item.StackCount += Count;
            OnInventoryUpdated.Broadcast();
            return;
        }
    }

    // 2. Otherwise, find the first empty slot and fill it in place
    for (FInventoryItem& Item : InventoryItems)
    {
        UE_LOG(LogTemp, Warning, TEXT("Checking slot — ItemDef is %s"), Item.ItemDef ? TEXT("NOT NULL") : TEXT("NULL"));
        if (Item.ItemDef == nullptr)
        {
            Item.ItemDef = Def;
            Item.StackCount = Count;
            UE_LOG(LogTemp, Warning, TEXT("Item placed successfully. ItemDef = %s"), *Def->GetName());
            OnInventoryUpdated.Broadcast();
            return;
        }
    }

    // 3. No stack match, no empty slot — inventory is full, silently fail
    // (Optional: could add a UFUNCTION event here for "inventory full" UI feedback later)
}

void UInventoryComponent::RemoveItem(int32 Index, int32 Count)
{
    if (InventoryItems.IsValidIndex(Index) && Count > 0)
    {
        InventoryItems[Index].StackCount -= Count;

        if (InventoryItems[Index].StackCount <= 0)
        {
            // Clear the slot in place — do NOT RemoveAt, that would shift every other slot
            InventoryItems[Index] = FInventoryItem();
        }

        OnInventoryUpdated.Broadcast();
    }
}

void UInventoryComponent::UseItem(int32 Index)
{
    if (InventoryItems.IsValidIndex(Index))
    {
        UItemDefinition* ItemDef = InventoryItems[Index].ItemDef;
        // NOTE: superseded by InventoryRulesSubsystem::RequestExecuteAction — left as stub
    }
}

bool UInventoryComponent::MoveItem(int32 FromIndex, int32 ToIndex)
{
    if (!InventoryItems.IsValidIndex(FromIndex) || !InventoryItems.IsValidIndex(ToIndex) || FromIndex == ToIndex)
    {
        return false;
    }

    InventoryItems.Swap(FromIndex, ToIndex);
    OnInventoryUpdated.Broadcast();
    return true;
}

bool UInventoryComponent::GetItemAtIndex(int32 Index, FInventoryItem& OutItem) const
{
    if (InventoryItems.IsValidIndex(Index))
    {
        OutItem = InventoryItems[Index];
        return true;
    }
    return false;
}