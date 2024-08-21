#include "Actors/InteractableActor.h"

#include "InteractionGameplayTags.h"
#include "InteractionLog.h"
#include "Components/InteractionTarget.h"
#include "Components/InteractorManager.h"
#include "Components/StateTreeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Objects/InteractionFinish.h"
#include "UserInterface/UW_InteractionTarget.h"

AInteractableActor::AInteractableActor()
{
	// 매 프레임마다 Tick()을 호출하도록 이 액터를 설정합니다.  필요하지 않은 경우 이 기능을 꺼서 성능을 향상시킬 수 있습니다.
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
	

	UInteractionManager* InteractingManager = FindInteractingManager();
	if (IsValid(InteractingManager))
	{
		ClientCheckPressedKey(InteractingManager);
	}
}

void AInteractableActor::TryTakeAction(UInteractionManager* InteractingManager)
{
	TArray<FKey> InteractionKeys;
	if (!InteractingManager->GetInteractionKeys(InteractionKeys))
	{
		LOG_WARNING_AND_SCREEN(5.0f, TEXT("상호작용 키가 존재하지 않습니다"));
		return;
	}

	for (FKey& Key : InteractionKeys)
	{
		if (InteractingManager->OwnerController->WasInputKeyJustPressed(Key))
		{
			LastPressedKey = Key;
			break;
		}
	}
	
	OwnedInteractionWidget = InteractingManager->FindWidgetByInteractionTarget(InteractionTarget);

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

	if (InteractingManager->OwnerController->WasInputKeyJustPressed(LastPressedKey))
		bKeyJustPressed = true;

	if (InteractingManager->OwnerController->WasInputKeyJustReleased(LastPressedKey))
		bKeyJustPressed = false;
}

UInteractionManager* AInteractableActor::FindInteractingManager()
{
	for (AController* AssignedInteractor : InteractionTarget->AssignedControllers)
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
	if (bKeyJustPressed)
	{
		ServerOnInteractionBegin(InteractingManager);
		OwnedInteractionWidget->UpdateInteractionText(true, EInteractionState::Done);
		OwnedInteractionWidget->PlayInteractionCompletedAnimation(EInteractionResult::Completed);
		ServerOnInteractionFinished(InteractingManager, EInteractionResult::Completed);
	}
}

void AInteractableActor::HandleHoldInteraction(UInteractionManager* InteractingManager)
{
	if (InteractingManager->OwnerController->IsInputKeyDown(LastPressedKey) && bKeyJustPressed)
	{
		if (CurrentHoldTime == 0.0f)
		{
			OwnedInteractionWidget->UpdateInteractionText(false, EInteractionState::Interacting);
			ServerOnInteractionBegin(InteractingManager);
		}

		double DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this) + CurrentHoldTime;
		CurrentHoldTime = DeltaTime;

		float NormalizedHoldTime = UKismetMathLibrary::NormalizeToRange(DeltaTime, 0.0f, InteractionTarget->HoldSeconds);
		float InteractionRatio = FMath::Min(NormalizedHoldTime, 1.0f);

		OwnedInteractionWidget->SetInteractionPercent(InteractionRatio);
		OnInteractionUpdated(InteractingManager, InteractionRatio, 0);

		if (InteractionRatio == 1.0f)
		{
			CurrentHoldTime = 0.0f;
			OwnedInteractionWidget->PlayInteractionCompletedAnimation(EInteractionResult::Completed);
			ServerOnInteractionFinished(InteractingManager, EInteractionResult::Completed);
		}
	}
	else
	{
		if (CurrentHoldTime == 0.0f)
			return;

		if (InteractionTarget->bCoolDownEnabled)
		{
			CurrentHoldTime = FMath::Max(CurrentHoldTime - UGameplayStatics::GetWorldDeltaSeconds(this), 0.0f);

			float NormalizedHoldTime = UKismetMathLibrary::NormalizeToRange(CurrentHoldTime, 0.0f, InteractionTarget->HoldSeconds);
			float InteractionRatio = FMath::Max(NormalizedHoldTime, 0.0f);
			OwnedInteractionWidget->SetInteractionPercent(InteractionRatio);

			if (CurrentHoldTime == 0.0f)
			{
				OwnedInteractionWidget->PlayInteractionCompletedAnimation(EInteractionResult::Canceled);
				ServerOnInteractionFinished(InteractingManager, EInteractionResult::Canceled);
			}
			else
			{
				OnInteractionUpdated(InteractingManager, InteractionRatio, 0);
				if (InteractionTarget->CancelOnRelease())
				{
					ServerOnInteractionFinished(InteractingManager, EInteractionResult::Canceled);
				}
			}
		}
		else
		{
			CurrentHoldTime = 0.0f;
			OwnedInteractionWidget->SetInteractionPercent(0.0f);
			OwnedInteractionWidget->PlayInteractionCompletedAnimation(EInteractionResult::Canceled);
			ServerOnInteractionFinished(InteractingManager, EInteractionResult::Canceled);
		}
	}
}

void AInteractableActor::HandleRepeatInteraction(UInteractionManager* InteractingManager)
{
	if (InteractingManager->OwnerController->WasInputKeyJustPressed(LastPressedKey))
	{
		RepeatCooldown = InteractionTarget->RepeatCoolDown;

		if (Repeated == 0)
		{
			OwnedInteractionWidget->UpdateInteractionText(false, EInteractionState::Interacting);
			ServerOnInteractionBegin(InteractingManager);
		}

		Repeated = Repeated + 1;
		float NormalizeRepeatTime = UKismetMathLibrary::NormalizeToRange(Repeated, 0.0f, InteractionTarget->RepeatCount);
		float RatioValue = FMath::Min(NormalizeRepeatTime, 1.0f);

		OnInteractionUpdated(InteractingManager, RatioValue, Repeated);
		OwnedInteractionWidget->SetInteractionPercent(RatioValue);

		if (Repeated == InteractionTarget->RepeatCount)
		{
			Repeated = 0;
			OwnedInteractionWidget->PlayInteractionCompletedAnimation(EInteractionResult::Completed);
			ServerOnInteractionFinished(InteractingManager, EInteractionResult::Completed);
		}
		else
		{
			OwnedInteractionWidget->PlayInteractionUpdateAnimation();
		}
	}

	if (Repeated != 0 && InteractionTarget->bCoolDownEnabled)
	{
		RepeatCooldown = FMath::Max(RepeatCooldown - UGameplayStatics::GetWorldDeltaSeconds(this), 0.0f);

		if (RepeatCooldown == 0.0f)
		{
			OwnedInteractionWidget->PlayInteractionCompletedAnimation(EInteractionResult::Canceled);
			Repeated = Repeated - 1;

			if (Repeated == 0)
			{
				OwnedInteractionWidget->SetInteractionPercent(0.0f);
				OwnedInteractionWidget->PlayInteractionCompletedAnimation(EInteractionResult::Canceled);
				ServerOnInteractionFinished(InteractingManager, EInteractionResult::Canceled);
				OwnedInteractionWidget->ResetProgress();
				OwnedInteractionWidget->UpdateInteractionText(false, EInteractionState::Waiting);
			}
			else
			{
				RepeatCooldown = InteractionTarget->RepeatCoolDown;
			}

			OnInteractionUpdated(InteractingManager, 0.0f, Repeated);
		}
		else
		{
			float NormalizeRepeatTime = UKismetMathLibrary::NormalizeToRange(Repeated, 0.0f, InteractionTarget->RepeatCount);
			float RatioValue = FMath::Min(NormalizeRepeatTime, 1.0f);

			float DivideValue = (InteractionTarget->RepeatCoolDown * Repeated) / RatioValue;
			float NewPercent = OwnedInteractionWidget->CurrentPercent - (UGameplayStatics::GetWorldDeltaSeconds(this) * (1.0f / DivideValue));
			OwnedInteractionWidget->SetInteractionPercent(NewPercent);
		}
	}
}

void AInteractableActor::OnInteractionUpdated(UInteractionManager* InteractingManager, float InAlpha, int32 InRepeated)
{
	if (InteractionTarget->OnInteractionUpdated.IsBound())
		InteractionTarget->OnInteractionUpdated.Broadcast(InAlpha, InRepeated, InteractingManager->OwnerController->GetPawn());

	FStateTreeEvent SendEvent;
	SendEvent.Tag = InteractionGameTags::Interaction_Update;
	StateTreeComponent->SendStateTreeEvent(SendEvent);

	// 현재 환경이 스탠드얼론(싱글플레이어) 모드가 아닌 경우, 서버와 상호작용 상태를 동기화합니다.
	if (!UKismetSystemLibrary::IsStandalone(this))
	{
		ServerOnInteractionUpdated(InteractingManager, InAlpha, InRepeated, InteractingManager->OwnerController->GetPawn());
	}
}

void AInteractableActor::SetEnableInteractivity_Implementation(bool bIsEnable)
{
	SetActorTickEnabled(bIsEnable);
	
}

void AInteractableActor::ResetData_Implementation()
{
	ClientResetData();
}

void AInteractableActor::SendEvent_Implementation(FStateTreeEvent NewEvent)
{
	StateTreeComponent->SendStateTreeEvent(NewEvent);
}

void AInteractableActor::ClientCheckPressedKey_Implementation(UInteractionManager* InteractingManager)
{
	TryTakeAction(InteractingManager);
}

void AInteractableActor::ClientResetData_Implementation()
{
	bKeyJustPressed = false;
	LastPressedKey = FKey();
	CurrentHoldTime = 0.0f;
	Repeated = 0;
}

void AInteractableActor::ServerOnInteractionBegin_Implementation(UInteractionManager* InteractingManager)
{
	if (InteractionTarget->OnInteractionBegin.IsBound())
		InteractionTarget->OnInteractionBegin.Broadcast(InteractingManager->OwnerController->GetPawn());
	
	FStateTreeEvent SendEvent;
	SendEvent.Tag = InteractionGameTags::Interaction_Begin;
	StateTreeComponent->SendStateTreeEvent(SendEvent);

	InteractingManager->bIsInteracting = true;
}

void AInteractableActor::ServerOnInteractionUpdated_Implementation(UInteractionManager* InteractingManager, float InAlpha, int32 InRepeated, APawn* InteractionPawn)
{
	if (InteractionTarget->OnInteractionUpdated.IsBound())
		InteractionTarget->OnInteractionUpdated.Broadcast(InAlpha, InRepeated, InteractionPawn);

	FStateTreeEvent SendEvent;
	SendEvent.Tag = InteractionGameTags::Interaction_Update;
	StateTreeComponent->SendStateTreeEvent(SendEvent);
}

void AInteractableActor::ServerOnInteractionFinished_Implementation(UInteractionManager* InteractingManager, EInteractionResult InteractionResult)
{
	InteractionTarget->ApplyFinishMethod(InteractingManager, InteractionResult);
}
