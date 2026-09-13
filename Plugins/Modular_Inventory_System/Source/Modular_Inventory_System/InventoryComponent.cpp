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
    if (!Def || Count <= 0)
    {
        return;
    }

    const int32 MaxStack = FMath::Max(1, Def->MaxStackSize);
    int32 Remaining = Count;

    // 1. Top up existing stacks of the same item, up to their max
    for (FInventoryItem& Item : InventoryItems)
    {
        if (Remaining <= 0) break;
        if (Item.ItemDef == Def && Item.StackCount < MaxStack)
        {
            const int32 Space = MaxStack - Item.StackCount;
            const int32 ToAdd = FMath::Min(Space, Remaining);
            Item.StackCount += ToAdd;
            Remaining -= ToAdd;
        }
    }

    // 2. Place any remainder into empty slots (one new stack per slot, capped)
    for (FInventoryItem& Item : InventoryItems)
    {
        if (Remaining <= 0) break;
        if (Item.ItemDef == nullptr)
        {
            const int32 ToAdd = FMath::Min(MaxStack, Remaining);
            Item.ItemDef = Def;
            Item.StackCount = ToAdd;
            Remaining -= ToAdd;
        }
    }

    // 3. If Remaining > 0 here, inventory couldn't fit everything (full). Silently dropped for now.
    //    (Future: return leftover count / fire an "inventory full" event for UI feedback.)

    OnInventoryUpdated.Broadcast();
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

bool UInventoryComponent::TransferItemTo(int32 FromIndex, UInventoryComponent* DestInv, int32 ToIndex)
{
    // Step 0 — same component → degrade to a plain move/swap
    if (DestInv == this)
    {
        return MoveItem(FromIndex, ToIndex);
    }

    // Step 1 — validate source
    if (!DestInv || !InventoryItems.IsValidIndex(FromIndex))
    {
        return false;
    }

    FInventoryItem& Source = InventoryItems[FromIndex];
    if (Source.ItemDef == nullptr || Source.StackCount <= 0)
    {
        return false;
    }

    UItemDefinition* Def = Source.ItemDef;
    const int32 MaxStack = FMath::Max(1, Def->MaxStackSize);
    bool bMovedAnything = false;

    // Step 2 — merge into existing stacks of the same item in the destination
    if (MaxStack > 1)
    {
        for (FInventoryItem& DestItem : DestInv->InventoryItems)
        {
            if (Source.StackCount <= 0) break;
            if (DestItem.ItemDef == Def && DestItem.StackCount < MaxStack)
            {
                const int32 Space = MaxStack - DestItem.StackCount;
                const int32 Moved = FMath::Min(Space, Source.StackCount);
                DestItem.StackCount += Moved;   // +Moved to dest
                Source.StackCount -= Moved;    // -Moved from source (same number)
                bMovedAnything = true;
            }
        }
    }

    // Step 3 — place remainder into first empty destination slot
    if (Source.StackCount > 0)
    {
        for (FInventoryItem& DestItem : DestInv->InventoryItems)
        {
            if (DestItem.ItemDef == nullptr)
            {
                DestItem.ItemDef = Def;
                DestItem.StackCount = Source.StackCount;
                Source.StackCount = 0;
                bMovedAnything = true;
                break;
            }
        }
    }

    // Step 4 — nothing fit: if a specific target slot was given and both are single items, swap
    if (Source.StackCount > 0 && DestInv->InventoryItems.IsValidIndex(ToIndex))
    {
        FInventoryItem& DestSlot = DestInv->InventoryItems[ToIndex];
        if (Source.StackCount == 1 && DestSlot.StackCount == 1 && DestSlot.ItemDef != nullptr)
        {
            const FInventoryItem Temp = DestSlot;
            DestSlot = Source;
            Source = Temp;
            bMovedAnything = true;
        }
    }

    // Clear source slot if fully emptied
    if (Source.StackCount <= 0)
    {
        Source = FInventoryItem();
    }

    // Fire updates on BOTH components if anything changed
        // Fire updates on BOTH components if anything changed
    if (bMovedAnything)
    {
        OnInventoryUpdated.Broadcast();
        DestInv->OnInventoryUpdated.Broadcast();
    }

    // --- DEBUG: total item accounting to catch dupe/loss ---
    int32 SourceTotal = 0;
    for (const FInventoryItem& I : InventoryItems)
        if (I.ItemDef) SourceTotal += I.StackCount;
    int32 DestTotal = 0;
    for (const FInventoryItem& I : DestInv->InventoryItems)
        if (I.ItemDef) DestTotal += I.StackCount;

    UE_LOG(LogTemp, Warning, TEXT("Transfer result: moved=%s | SOURCE total items=%d | DEST total items=%d | GRAND TOTAL=%d"),
        bMovedAnything ? TEXT("YES") : TEXT("NO"), SourceTotal, DestTotal, SourceTotal + DestTotal);

    return bMovedAnything;
}   