#include "InventoryManagerComponent.h"
#include "InventoryComponent.h"
#include "GameFramework/Actor.h"	
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "ItemDefinition.h"
#include "InventoryScreenInterface.h"	
#include "InventoryRulesSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

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
	if (!IsValid(InventoryScreen))
	{
		return;
	}

	APlayerController* Controller = InventoryScreen->GetOwningPlayer();
	if (!IsValid(Controller))
	{
		return;
	}

	if (InventoryScreen->IsVisible())
	{
		InventoryScreen->SetVisibility(ESlateVisibility::Collapsed);
		Controller->SetInputMode(FInputModeGameOnly());
		Controller->SetShowMouseCursor(false);
	}
	else
	{
		InventoryScreen->SetVisibility(ESlateVisibility::Visible);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		Controller->SetInputMode(InputMode);
		Controller->SetShowMouseCursor(true);
	}
}


void UInventoryManagerComponent::SetNearbyItem(AActor* Item)
{
	NearbyItem = Item;
}


void UInventoryManagerComponent::RequestPickup()
{
	// migrated logic goes here (Step 4)
	if (!IsValid(Inventory)) return;

	AActor* Item = NearbyItem.Get();
	if (!IsValid(Item)) return;

	if (!Item->Implements<UInventoryPickupInterface>()) return;
	UItemDefinition* ItemDef = IInventoryPickupInterface::Execute_GetPickupItemDefinition(Item);
	if (!IsValid(ItemDef)) return;

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	if (!IsValid(GameInstance)) return;

	
	UInventoryRulesSubsystem* Rules = GameInstance->GetSubsystem<UInventoryRulesSubsystem>();
	if (!IsValid(Rules)) return;
	
	int32 added = Rules->RequestPickupItem(Inventory, ItemDef , 1);

	if (added == 1) {
		Item->Destroy();
		NearbyItem.Reset();
	}
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
