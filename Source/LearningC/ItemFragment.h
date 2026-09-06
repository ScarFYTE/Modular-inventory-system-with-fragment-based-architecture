#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemFragment.generated.h"

// This makes the fragment abstract, editable inside a data asset, and instanced.
UCLASS(Abstract, EditInlineNew, DefaultToInstanced,Blueprintable,BlueprintType)
class LEARNINGC_API UItemFragment : public UObject
{
    GENERATED_BODY()
};