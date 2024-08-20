#include "Objects/DestroyOnFinish.h"

#include "Components/InteractionTarget.h"


void UDestroyOnFinish::Execute(EInteractionResult InteractionResult)
{
	if (InteractionResult == EInteractionResult::Completed && bIsComplete ||
		InteractionResult == EInteractionResult::Canceled && bIsCancel ||
		InteractionResult == EInteractionResult::None)
	{
		InteractionTarget->GetOwner()->Destroy();
	}
}
