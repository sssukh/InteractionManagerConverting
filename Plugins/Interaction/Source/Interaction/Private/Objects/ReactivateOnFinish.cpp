#include "Objects/ReactivateOnFinish.h"

#include "InteractionGameplayTags.h"
#include "Actors/InteractableActor.h"
#include "Components/InteractionTarget.h"
#include "Components/InteractorManager.h"
#include "Components/SphereComponent.h"
#include "Components/StateTreeComponent.h"
#include "Interfaces/Interface_Interaction.h"
#include "Kismet/KismetSystemLibrary.h"


void UReactivateOnFinish::Execute(EInteractionResult InteractionResult)
{
	InteractionTarget->LastInteractedTime = UKismetSystemLibrary::GetGameTimeInSeconds(this);

	if (InteractionResult == EInteractionResult::Completed && bIsComplete ||
		InteractionResult == EInteractionResult::Canceled && bIsCancel ||
		InteractionResult == EInteractionResult::None)
	{
		for (AController* AssignedController : InteractionTarget->AssignedControllers)
		{
			UInteractionManager* AssignedInteractionManager = AssignedController->GetComponentByClass<UInteractionManager>();
			if (!AssignedInteractionManager) continue;

			AssignedInteractionManager->PendingTargets.AddUnique(InteractionTarget);
			AssignedInteractionManager->ServerUpdatePointOfInterests(false, InteractionTarget);
			AssignedInteractionManager->ServerUpdateInteractionTargets(false, InteractionTarget);
		}

		InteractionTarget->bInteractionEnabled = false;

		if (UKismetSystemLibrary::K2_IsValidTimerHandle(PendingTarget_TimerHandle)) return;

		GetWorld()->GetTimerManager().SetTimer(PendingTarget_TimerHandle, this, &UReactivateOnFinish::CheckForPendingTargets, PendingTargetCheckInterval, true);
	}
	else
	{
		if (UKismetSystemLibrary::K2_IsValidTimerHandle(PendingTarget_TimerHandle))
			UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, PendingTarget_TimerHandle);
	}
}

void UReactivateOnFinish::CheckForPendingTargets()
{
	double TimeSeconds = UKismetSystemLibrary::GetGameTimeInSeconds(this);

	double ElapsedTime = FMath::Abs(TimeSeconds - InteractionTarget->LastInteractedTime);

	if (ElapsedTime >= ReActivationDuration)
	{
		InteractionManager->PendingTargets.Remove(InteractionTarget);

		InteractionTarget->bInteractionEnabled = true;

		// 컨트롤러가 실제로 재활성화된 대상과 겹치는지 확인하고 플레이어가 현재 겹치는 구성 요소를 캐시합니다.
		if (InteractionTarget->AssignedControllers.Contains(InteractionController))
		{
			TArray<UPrimitiveComponent*> PawnOverlappingComponents;
			InteractionController->GetPawn()->GetOverlappingComponents(PawnOverlappingComponents);

			if (PawnOverlappingComponents.Contains(InteractionTarget->InnerZone))
			{
				InteractionManager->ServerUpdatePointOfInterests(true, InteractionTarget);
				InteractionManager->ServerUpdateInteractionTargets(true, InteractionTarget);
			}
			else if (PawnOverlappingComponents.Contains(InteractionTarget->OuterZone))
			{
				InteractionManager->ServerUpdatePointOfInterests(true, InteractionTarget);
			}
		}
		
		if (InteractionTarget->OnInteractionUpdated.IsBound())
			InteractionTarget->OnInteractionReactivated.Broadcast(InteractionPawn);

		UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, PendingTarget_TimerHandle);
	}
}
