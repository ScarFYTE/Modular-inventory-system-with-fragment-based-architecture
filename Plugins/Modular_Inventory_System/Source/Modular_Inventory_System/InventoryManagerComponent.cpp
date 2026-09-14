#include "InventoryManagerComponent.h"
#include "InventoryComponent.h"
#include "GameFramework/Actor.h"	
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "InventoryScreenInterface.h"	

UInventoryManagerComponent::UInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* OwnerActor = GetOwner())
	{
		Inventory = OwnerActor->FindComponentByClass<UInventoryComponent>();
	}

	// Bind refresh to inventory changes
	if (Inventory)
	{
		Inventory->OnInventoryUpdated.AddDynamic(this, &UInventoryManagerComponent::HandleInventoryUpdated);
	}

	if (InventoryScreenClass && Inventory)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			InventoryScreen = CreateWidget<UUserWidget>(PC, InventoryScreenClass);
			if (InventoryScreen)
			{
				InventoryScreen->AddToViewport();
				InventoryScreen->SetVisibility(ESlateVisibility::Collapsed);

				if (InventoryScreen->Implements<UInventoryScreenInterface>())
				{
					IInventoryScreenInterface::Execute_InitializeScreen(InventoryScreen, Inventory);
				}
			}
		}
	}
}

void UInventoryManagerComponent::ToggleInventory()
{
	// migrated logic goes here (Step 3)
}

void UInventoryManagerComponent::RequestPickup()
{
	// migrated logic goes here (Step 4)
}

void UInventoryManagerComponent::RequestInteract()
{
	// migrated logic goes here (Phase 4)
}



void UInventoryManagerComponent::HandleInventoryUpdated()
{
	if (InventoryScreen && InventoryScreen->Implements<UInventoryScreenInterface>())
	{
		IInventoryScreenInterface::Execute_RefreshScreen(InventoryScreen);
	}
}