#pragma once

#include "CoreMinimal.h"
#include "InteractionFinish.h"
#include "DeactivateOnFinish.generated.h"

/**
 * 
 */
UCLASS()
class INTERACTION_API UDeactivateOnFinish : public UInteractionFinish
{
	GENERATED_BODY()

public:
	virtual void Execute(EInteractionResult InteractionResult) override;
};
