#include "Objects/InteractionFinish.h"

#include "Actors/InteractableActor.h"
#include "Components/InteractionTarget.h"
#include "Components/InteractorManager.h"

void UInteractionFinish::InitializeOnFinish(UInteractionManager* CallingInteractionManager, UInteractionTarget* InInteractionTarget)
{
	InteractionTarget = InInteractionTarget;
	InteractableActor = InInteractionTarget->GetOwner();

	InteractionManager = CallingInteractionManager;
	InteractionController = CallingInteractionManager->OwnerController;
	InteractionPawn = InteractionController->GetPawn();
}

void UInteractionFinish::Execute(EInteractionResult InteractionResult)
{
}
