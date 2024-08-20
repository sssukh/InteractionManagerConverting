#include "Actors/InteractableActor.h"

#include "InteractionGameplayTags.h"
#include "InteractionLog.h"
#include "Components/InteractionTarget.h"
#include "Components/InteractorManager.h"
#include "Components/StateTreeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UserInterface/UW_InteractionTarget.h"

AInteractableActor::AInteractableActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultScene"));
	SetRootComponent(DefaultSceneComponent);

	MarkerTarget = CreateDefaultSubobject<USceneComponent>(TEXT("MarkerTarget"));
	MarkerTarget->SetupAttachment(RootComponent);

	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTree"));

	InteractionTarget = CreateDefaultSubobject<UInteractionTarget>(TEXT("InteractionTarget"));
}

void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// UInteractionManager* InteractingManager = FindInteractingManager();
	// if (IsValid(InteractingManager) && InteractionTarget->bInteractionEnabled)
	// {
	// 	TryTakeAction(InteractingManager);
	// }
}

void AInteractableActor::TryTakeAction(UInteractionManager* InteractingManager)
{
	if (InteractionTarget->InteractionType == EInteractionType::Tap)
	{
		HandleTapInteraction(InteractingManager);
	}
	else if (InteractionTarget->InteractionType == EInteractionType::Hold)
	{
		HandleHoldInteraction(InteractingManager);
	}
	else if (InteractionTarget->InteractionType == EInteractionType::Repeat)
	{
		HandleRepeatInteraction(InteractingManager);
	}
}

UInteractionManager* AInteractableActor::FindInteractingManager()
{
	for (AController* AssignedInteractor : InteractionTarget->AssignedInteractors)
	{
		UInteractionManager* AssignedInteractionManager = Cast<UInteractionManager>(AssignedInteractor->GetComponentByClass(UInteractionManager::StaticClass()));
		if (!AssignedInteractionManager) continue;

		if (AssignedInteractionManager->BestInteractionTarget == InteractionTarget)
		{
			return AssignedInteractionManager;
		}
	}
	return nullptr;
}

void AInteractableActor::HandleTapInteraction(UInteractionManager* InteractingManager)
{
	if (InteractingManager->bKeyJustPressed)
	{
		ServerOnInteractionBegin(InteractingManager);
		InteractingManager->CurrentInteractionMarker->UpdateInteractionText(true, EInteractionState::Done);
		InteractingManager->CurrentInteractionMarker->PlayInteractionCompletedAnimation(EInteractionResult::Completed);
		InteractingManager->ServerOnInteractionFinished(InteractionTarget, EInteractionResult::Completed);
	}
}

void AInteractableActor::HandleHoldInteraction(UInteractionManager* InteractingManager)
{
	// 키가 눌려있고, 키가 방금 눌렸는지 확인
	if (InteractingManager->IsHoldingKey())
	{
		// 상호작용이 처음 시작되었을 때
		if (CurrentHoldTime == 0.0f)
		{
			InteractingManager->CurrentInteractionMarker->UpdateInteractionText(false, EInteractionState::Interacting);
			InteractingManager->ServerOnInteractionBegin(InteractionTarget);
		}

		// 누적된 홀드 시간 계산
		double DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this) + CurrentHoldTime;
		CurrentHoldTime = DeltaTime;

		// 누적된 홀드 시간을 상호작용 완료 시간을 기준으로 정규화하여 비율 계산
		float NormalizedHoldTime = UKismetMathLibrary::NormalizeToRange(DeltaTime, 0.0f, InteractionTarget->HoldSeconds);
		float InteractionProgress = FMath::Min(NormalizedHoldTime, 1.0f);

		// 상호작용 진행 상태 업데이트
		InteractingManager->CurrentInteractionMarker->SetInteractionPercent(InteractionProgress);
		InteractingManager->OnInteractionUpdated(InteractionTarget, InteractionProgress, 0);

		// 상호작용이 완료되었을 때
		if (InteractionProgress == 1.0f)
		{
			CurrentHoldTime = 0.0f;
			InteractingManager->CurrentInteractionMarker->PlayInteractionCompletedAnimation(EInteractionResult::Completed);
			InteractingManager->ServerOnInteractionFinished(InteractionTarget, EInteractionResult::Completed);
		}
	}
	else
	{
		// 누적된 홀드 시간이 0이라면 처리할 것이 없음
		if (CurrentHoldTime == 0.0f)
			return;

		// 상호작용이 쿨다운이 활성화된 상태에서 중단되었을 때
		if (InteractionTarget->bCoolDownEnabled)
		{
			// 쿨다운 적용하여 홀드 시간 감소
			CurrentHoldTime = FMath::Max(CurrentHoldTime - UGameplayStatics::GetWorldDeltaSeconds(this), 0.0f);

			// 정규화된 홀드 시간과 비율 계산
			float NormalizedHoldTime = UKismetMathLibrary::NormalizeToRange(CurrentHoldTime, 0.0f, InteractionTarget->HoldSeconds);
			float InteractionProgress = FMath::Max(NormalizedHoldTime, 0.0f);
			InteractingManager->CurrentInteractionMarker->SetInteractionPercent(InteractionProgress);

			// 쿨다운으로 인해 홀드 시간이 0이 되면 상호작용 취소
			if (CurrentHoldTime == 0.0f)
			{
				InteractingManager->CurrentInteractionMarker->PlayInteractionCompletedAnimation(EInteractionResult::Canceled);
				InteractingManager->ServerOnInteractionFinished(InteractionTarget, EInteractionResult::Canceled);
			}
			else
			{
				// 상호작용이 계속 진행 중일 때 업데이트
				InteractingManager->OnInteractionUpdated(InteractionTarget, InteractionProgress, 0);
				if (InteractionTarget->CancelOnRelease())
				{
					InteractingManager->ServerOnInteractionFinished(InteractionTarget, EInteractionResult::Canceled);
				}
			}
		}
		else
		{
			// 쿨다운이 비활성화된 경우 모든 것을 재설정하고 상호작용을 취소
			CurrentHoldTime = 0.0f;
			InteractingManager->CurrentInteractionMarker->SetInteractionPercent(0.0f);
			InteractingManager->CurrentInteractionMarker->PlayInteractionCompletedAnimation(EInteractionResult::Canceled);
			InteractingManager->ServerOnInteractionFinished(InteractionTarget, EInteractionResult::Canceled);
		}
	}
}

void AInteractableActor::HandleRepeatInteraction(UInteractionManager* InteractingManager)
{
	if (InteractingManager->OwnerController->WasInputKeyJustPressed(InteractingManager->LastPressedKey))
	{
		RepeatCooldown = InteractionTarget->RepeatCoolDown;

		if (Repeated == 0)
		{
			InteractingManager->CurrentInteractionMarker->UpdateInteractionText(false, EInteractionState::Interacting);
			ServerOnInteractionBegin(InteractingManager);
		}

		Repeated = Repeated + 1;
		float NormalizeRepeatTime = UKismetMathLibrary::NormalizeToRange(Repeated, 0.0f, InteractionTarget->RepeatCount);
		float RatioValue = FMath::Min(NormalizeRepeatTime, 1.0f);

		InteractingManager->OnInteractionUpdated(InteractionTarget, RatioValue, Repeated);
		InteractingManager->CurrentInteractionMarker->SetInteractionPercent(RatioValue);

		if (Repeated == InteractionTarget->RepeatCount)
		{
			Repeated = 0;
			InteractingManager->CurrentInteractionMarker->PlayInteractionCompletedAnimation(EInteractionResult::Completed);
			InteractingManager->ServerOnInteractionFinished(InteractionTarget, EInteractionResult::Completed);
		}
		else
		{
			InteractingManager->CurrentInteractionMarker->PlayInteractionUpdateAnimation();
		}
	}


	if (Repeated != 0 && InteractionTarget->bCoolDownEnabled)
	{
		RepeatCooldown = FMath::Max(RepeatCooldown - UGameplayStatics::GetWorldDeltaSeconds(this), 0.0f);

		if (RepeatCooldown == 0.0f)
		{
			InteractingManager->CurrentInteractionMarker->PlayInteractionCompletedAnimation(EInteractionResult::Canceled);
			Repeated = Repeated - 1;

			if (Repeated == 0)
			{
				InteractingManager->CurrentInteractionMarker->SetInteractionPercent(0.0f);
				InteractingManager->CurrentInteractionMarker->PlayInteractionCompletedAnimation(EInteractionResult::Canceled);
				InteractingManager->ServerOnInteractionFinished(InteractionTarget, EInteractionResult::Canceled);
				InteractingManager->CurrentInteractionMarker->ResetProgress();
				InteractingManager->CurrentInteractionMarker->UpdateInteractionText(false, EInteractionState::Waiting);
			}
			else
			{
				RepeatCooldown = InteractionTarget->RepeatCoolDown;
			}

			InteractingManager->OnInteractionUpdated(InteractionTarget, 0.0f, Repeated);
		}
		else
		{
			float NormalizeRepeatTime = UKismetMathLibrary::NormalizeToRange(Repeated, 0.0f, InteractionTarget->RepeatCount);
			float RatioValue = FMath::Min(NormalizeRepeatTime, 1.0f);

			float DivideValue = (InteractionTarget->RepeatCoolDown * Repeated) / RatioValue;
			float NewPercent = InteractingManager->CurrentInteractionMarker->CurrentPercent - (UGameplayStatics::GetWorldDeltaSeconds(this) * (1.0f / DivideValue));
			InteractingManager->CurrentInteractionMarker->SetInteractionPercent(NewPercent);
		}
	}
}

void AInteractableActor::SendEvent_Implementation(FStateTreeEvent NewEvent)
{
	// FInteractionPayLoad NewPayLoad =  static_cast<FInteractionPayLoad>(NewEvent.Payload);
	StateTreeComponent->SendStateTreeEvent(NewEvent);


	if (NewEvent.Tag == InteractionGameTags::Interaction_Begin)
	{
		InteractionTarget->OnInteractionBeginEvent(FindInteractingManager()->OwnerController->GetPawn());
	}
}


void AInteractableActor::ServerOnInteractionBegin_Implementation(UInteractionManager* InteractingManager)
{
	FStateTreeEvent SendEvent;
	SendEvent.Tag = InteractionGameTags::Interaction_Type_Food;
	StateTreeComponent->SendStateTreeEvent(SendEvent);
	
	InteractingManager->bIsInteracting = true;
}
