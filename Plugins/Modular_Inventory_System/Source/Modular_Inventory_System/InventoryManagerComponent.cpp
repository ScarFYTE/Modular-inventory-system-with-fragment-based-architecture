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
		TArray<UInventoryComponent*> InventoryComponents;
		OwnerActor->GetComponents<UInventoryComponent>(InventoryComponents);
		
		if(InventoryComponents.Num() <= 0 )
		{
			UE_LOG(LogTemp, Warning, TEXT("No InventoryComponent found on %s"), *OwnerActor->GetName());
		}

		// Find the inventory component with the matching name

		else if (InventoryName.IsNone()) {
			if (InventoryComponents.Num() == 1)
			{
				Inventory = InventoryComponents[0];
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Multiple InventoryComponents found on %s"), *OwnerActor->GetName());
			}
		}
		else {
			TArray<UInventoryComponent*> InventoryComponentsFound;
			for (UInventoryComponent* InvComp : InventoryComponents)
			{
				if (InvComp && InvComp->ComponentHasTag(InventoryName))
				{
					InventoryComponentsFound.Add(InvComp);
				}
			}
			if (InventoryComponentsFound.Num() > 0) {
				if (InventoryComponentsFound.Num() == 1) {
					Inventory = InventoryComponentsFound[0];
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Multiple Inventory Components that share the Intended Tag Found on %s"), *OwnerActor->GetName());
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("No Inventory Component with the Intended Tag Found on %s"), *OwnerActor->GetName());
			}
		}
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
