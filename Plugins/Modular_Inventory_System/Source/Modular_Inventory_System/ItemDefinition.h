// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemFragment.h"
#include "ItemDefinition.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MODULAR_INVENTORY_SYSTEM_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Item")
		TArray<UItemFragment*> Fragments;

	template<class T>
	T* FindFragmentByClass() const {
		for (UItemFragment* Fragment : Fragments) {
			if (T* MatchingFragment = Cast<T>(Fragment)) {
				return MatchingFragment;
			}
		}
		return nullptr;
	}
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item", meta = (DisplayName = "Find Fragment By Class", DeterminesOutputType = "FragmentClass"))
	UItemFragment* FindFragmentByClass_BP(TSubclassOf<UItemFragment> FragmentClass) const
	{
		if (!FragmentClass) return nullptr;

		for (UItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
		return nullptr;
	}
};
