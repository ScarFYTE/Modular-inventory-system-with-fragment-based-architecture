// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemDefinition.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UItemDefinition* ItemDef = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 StackCount = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MODULAR_INVENTORY_SYSTEM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 SlotCount = 20;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(UItemDefinition* ItemDef, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(int32 Index, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItem(int32 FromIndex, int32 ToIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool GetItemAtIndex(int32 Index, FInventoryItem& OutItem) const;

	// Transfers a stack (or partial) from a slot in this component into DestInv.
	// ToIndex = -1 means "find best fit"; a valid index means "target that specific slot".
	// Returns true if anything moved.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TransferItemTo(int32 FromIndex, UInventoryComponent* DestInv, int32 ToIndex = -1);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryItem> InventoryItems;

};
