

#include "DelectCode.h"





//
// void UInteractionManager::OnInteractionTargetReActivated(UInteractionTarget* InteractionTarget)
// {
// 	PendingTargets.Remove(InteractionTarget);
// 	InteractionTarget->bInteractionEnabled = true;
//
// 	if (InteractionTarget->AssignedInteractors.Contains(OwnerController))
// 	{
// 		TArray<UPrimitiveComponent*> PawnOverlappingComponents;
// 		OwnerController->GetPawn()->GetOverlappingComponents(PawnOverlappingComponents);
//
//
// 		if (PawnOverlappingComponents.Contains(InteractionTarget->InnerZone))
// 		{
// 			ServerUpdatePointOfInterests(true, InteractionTarget);
// 			ServerUpdateInteractionTargets(true, InteractionTarget);
// 		}
// 		else if (PawnOverlappingComponents.Contains(InteractionTarget->OuterZone))
// 		{
// 			ServerUpdatePointOfInterests(true, InteractionTarget);
// 		}
// 	}
//
// 	if (InteractionTarget->OnInteractionReactivated.IsBound())
// 		InteractionTarget->OnInteractionReactivated.Broadcast(OwnerController->GetPawn());
// }