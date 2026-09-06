#pragma once
#include "ItemFragment.h"
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "InventoryRulesSubsystem.generated.h"



class UInventoryComponent;
class AActor;

UCLASS()
class LEARNINGC_API UInventoryRulesSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * The universal request to drop an item.
	 * The UI calls this instead of spawning actors itself.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Rules")
	void RequestExecuteAction(UInventoryComponent* InventoryComp, int32 SlotIndex, UItemFragment* TargetFragment, AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Rules")
	void RequestDropItem(UInventoryComponent* InventoryComp, int32 SlotIndex, FTransform SpawnTransform, TSubclassOf<AActor> PickupActorClass, AActor*& OutSpawnedPickup, UItemDefinition*& OutItemDef);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Rules")
	bool RequestMoveItem(UInventoryComponent* InventoryComp, int32 FromIndex, int32 ToIndex);
};