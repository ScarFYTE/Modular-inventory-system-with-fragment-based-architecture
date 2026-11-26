#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagerComponent.generated.h"

class UInventoryComponent;
class UUserWidget;
class AActor;


UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class MODULAR_INVENTORY_SYSTEM_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> NearbyItem;
public:
	UInventoryManagerComponent();

	// The inventory this manager drives. Set in BeginPlay by finding it on the owning actor (Option B).
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	UInventoryComponent* Inventory = nullptr;

	// The screen widget class to spawn (set this in the editor per-project).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI")
	TSubclassOf<UUserWidget> InventoryScreenClass;

	// The spawned screen instance.
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI")
	UUserWidget* InventoryScreen = nullptr;

	// ---- Public functions the owning actor's input calls ----
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestPickup();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestInteract();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void SetNearbyItem(AActor* Item);

	UFUNCTION()
	void HandleInventoryUpdated();

protected:
	virtual void BeginPlay() override;
};