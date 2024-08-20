#include "Objects/DeactivateOnFinish.h"

#include "Components/InteractorManager.h"

void UDeactivateOnFinish::Execute(EInteractionResult InteractionResult)
{
	if (InteractionResult == EInteractionResult::Completed && bIsComplete ||
		InteractionResult == EInteractionResult::Canceled && bIsCancel ||
		InteractionResult == EInteractionResult::None)
	{
		InteractionManager->AddToDeactivatedTargets(InteractionTarget);
	}
}
