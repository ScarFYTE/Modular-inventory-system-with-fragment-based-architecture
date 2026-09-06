#include "InventoryRulesSubsystem.h"
#include "InventoryComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UInventoryRulesSubsystem::RequestDropItem(UInventoryComponent* InventoryComp, int32 SlotIndex, FTransform SpawnTransform, TSubclassOf<AActor> PickupActorClass, AActor*& OutSpawnedPickup, UItemDefinition*& OutItemDef)
{
    OutSpawnedPickup = nullptr;
    OutItemDef = nullptr;

    if (!InventoryComp || !PickupActorClass)
    {
        return;
    }

    FInventoryItem ItemToDrop;

    if (InventoryComp->GetItemAtIndex(SlotIndex, ItemToDrop))
    {
        OutItemDef = ItemToDrop.ItemDef;

        UWorld* World = GetWorld();
        if (World)
        {
            // 1. Offset the spawn location slightly in front/above the player
            FTransform AdjustedTransform = SpawnTransform;
            FVector SpawnLocation = AdjustedTransform.GetLocation() + (AdjustedTransform.GetRotation().GetForwardVector() * 80.0f) + FVector(0, 0, 20.0f);
            AdjustedTransform.SetLocation(SpawnLocation);

            // 2. Override collision so Unreal NEVER refuses to spawn it
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            OutSpawnedPickup = World->SpawnActor<AActor>(PickupActorClass, AdjustedTransform, SpawnParams);
        }

        InventoryComp->RemoveItem(SlotIndex, 1);
    }
}

void UInventoryRulesSubsystem::RequestExecuteAction(UInventoryComponent* InventoryComp, int32 SlotIndex, UItemFragment* TargetFragment, AActor* Instigator)
{
    if (!InventoryComp || !TargetFragment)
    {
        return;
    }

    FInventoryItem ItemToUse;
    if (InventoryComp->GetItemAtIndex(SlotIndex, ItemToUse) && ItemToUse.ItemDef)
    {
        UFunction* ExecuteFunc = TargetFragment->FindFunction(FName("ExecuteAction"));
        if (ExecuteFunc)
        {
            struct FExecuteActionParams
            {
                AActor* Instigator;
                uint8 Result;
            };

            FExecuteActionParams Params;
            Params.Instigator = Instigator;
            Params.Result = 0;

            TargetFragment->ProcessEvent(ExecuteFunc, &Params);

            if (Params.Result == 2)
            {
                InventoryComp->RemoveItem(SlotIndex, 1);
            }
        }
    }
}

bool UInventoryRulesSubsystem::RequestMoveItem(UInventoryComponent* InventoryComp, int32 FromIndex, int32 ToIndex)
{
    if (!InventoryComp)
    {
        return false;
    }

    return InventoryComp->MoveItem(FromIndex, ToIndex);
}
