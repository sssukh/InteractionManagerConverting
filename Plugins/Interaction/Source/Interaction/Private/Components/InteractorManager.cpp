#include "Components/InteractorManager.h"

#include "InteractionLog.h"

#include "EnhancedInputSubsystems.h"
#include "Actors/InteractableActor.h"
#include "Components/InteractionTarget.h"
#include "Components/PostProcessComponent.h"
#include "Components/SphereComponent.h"
#include "Interfaces/Interface_Interaction.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "UserInterface/UW_InteractionTarget.h"


UInteractionManager::UInteractionManager(): OwnerController(nullptr), PostProcessComponent(nullptr), bIsGamepad(false), bIsInteracting(false), RepeatCooldown(0),
                                            CurrentInteractionMarker(nullptr),
                                            Outline_DynamicMaterial(nullptr),
                                            BestInteractionTarget(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	// Material 경로로 CurrentMaterial 초기화
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Interaction/Environment/Materials/PostProcess/M_OutlineMaterial.M_OutlineMaterial"));
	if (MaterialFinder.Succeeded())
	{
		CurrentMaterial = MaterialFinder.Object;
	}

	// InputAction 경로로 InteractionInputAction 초기화
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Interaction/Input/InputAction_Interaction.InputAction_Interaction'"));
	if (InputActionFinder.Succeeded())
	{
		InteractionInputAction = InputActionFinder.Object;
	}

	WidgetScreenMargin = FMargin(100);
}

void UInteractionManager::BeginPlay()
{
	Super::BeginPlay();

	ConstructPlayerEssentials();

	ConstructPooledMarkerWidgets();
}

void UInteractionManager::ConstructPlayerEssentials()
{
	APlayerController* CastController = Cast<APlayerController>(GetOwner());

	if (!CastController)
	{
		LOG_WARNING_AND_SCREEN(5.0f, TEXT("APlayerController로 캐스팅에 실패했습니다"))
		return;
	}

	OwnerController = CastController;

	ConstructPostProcessComponent();

	GetWorld()->GetTimerManager().SetTimer(BeginUpdateKeys_TimerHandle, this, &UInteractionManager::UpdateInteractionKeys, 0.2f, false);
}

void UInteractionManager::ConstructPooledMarkerWidgets()
{
	if (!OwnerController->IsLocalController())
		return;

	if (!IsValid(InteractionTargetWidgetClass))
	{
		LOG_WARNING_AND_SCREEN(5.0f, TEXT("InteractionTargetWidgetClass를 설정해주세요"));
		return;
	}

	for (int i = 0; i < DefaultWidgetPoolSize; ++i)
	{
		UUW_InteractionTarget* NewInteractionWidget = CreateWidget<UUW_InteractionTarget>(OwnerController, InteractionTargetWidgetClass); // InteractionTargetWidgetClass를 사용하여 새로운 상호작용 타겟 위젯을 생성합니다.
		WidgetPool.AddUnique(NewInteractionWidget);
		NewInteractionWidget->AddToPlayerScreen();
	}
}

void UInteractionManager::ConstructPostProcessComponent()
{
	APawn* OwnerPawn = OwnerController->GetPawn();

	PostProcessComponent = NewObject<UPostProcessComponent>(OwnerPawn, UPostProcessComponent::StaticClass(), TEXT("PostProcessComponent"));
	PostProcessComponent->RegisterComponent();
	OwnerPawn->AddInstanceComponent(PostProcessComponent);

	Outline_DynamicMaterial = UMaterialInstanceDynamic::Create(CurrentMaterial, this);
	PostProcessComponent->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, Outline_DynamicMaterial));
}

void UInteractionManager::UpdateInteractionKeys()
{
	if (!OwnerController)
	{
		LOG_WARNING_AND_SCREEN(5.0f, TEXT("OwnerController이 존재하지 않습니다"));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = OwnerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	InteractionKeys = InputSubsystem->QueryKeysMappedToAction(InteractionInputAction);
	UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, BeginUpdateKeys_TimerHandle);
}

void UInteractionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (UInteractionTarget* Target : InteractionTargets)
	{
		GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::Green,FString::Printf(TEXT("%s 's CurrentHoldTime : %f"),*Target->GetName(),Cast<AInteractableActor>(Target->GetOwner())->CurrentHoldTime));
	}
	if (GetOwner()->HasAuthority())
	{
		UInteractionTarget* NearTarget = FindBestInteractable();
		UpdateBestInteractable(NearTarget);
		
		if (bDebug)
		{
			DrawDebugInteractor();
		}
	}
}

void UInteractionManager::DrawDebugInteractor()
{
	FString A = TEXT("Interaction Targets: ");
	FString B = UKismetStringLibrary::Conv_IntToString(InteractionTargets.Num());

	FString C = TEXT("\r\nPoint Of Interests: ");
	FString D = UKismetStringLibrary::Conv_IntToString(PointOfInterests.Num());

	FString E = TEXT("\r\nPending Targets: ");
	FString F = UKismetStringLibrary::Conv_IntToString(PendingTargets.Num());

	FString G = TEXT("\r\nMarker: ");
	FString H = UKismetSystemLibrary::GetDisplayName(CurrentInteractionMarker);

	FString I = TEXT("\r\nDeactivatedTargets: ");
	FString J = UKismetStringLibrary::Conv_IntToString(DeactivatedTargets.Num());

	FString DebugMessage = UKismetStringLibrary::Concat_StrStr(A, B);
	DebugMessage = UKismetStringLibrary::Concat_StrStr(DebugMessage, C);
	DebugMessage = UKismetStringLibrary::Concat_StrStr(DebugMessage, D);
	DebugMessage = UKismetStringLibrary::Concat_StrStr(DebugMessage, E);
	DebugMessage = UKismetStringLibrary::Concat_StrStr(DebugMessage, F);
	DebugMessage = UKismetStringLibrary::Concat_StrStr(DebugMessage, G);
	DebugMessage = UKismetStringLibrary::Concat_StrStr(DebugMessage, H);
	DebugMessage = UKismetStringLibrary::Concat_StrStr(DebugMessage, I);
	DebugMessage = UKismetStringLibrary::Concat_StrStr(DebugMessage, J);

	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, DebugMessage);
}

UInteractionTarget* UInteractionManager::FindBestInteractable()
{
	// 가장 높은 DOT Product 값을 저장할 변수입니다.
	float HighestDotProduct = 0.0f;

	// 가장 적합한 상호작용 타겟을 가리킬 포인터입니다.
	UInteractionTarget* BestTarget = nullptr;

	// InteractionTargets 배열에 있는 모든 타겟을 순회하며 최적의 타겟을 찾습니다.
	for (UInteractionTarget* InteractionTarget : InteractionTargets)
	{
		// 타겟이 유효하지 않거나 상호작용할 수 없는 상태라면 해당 타겟을 건너뜁니다.
		if (!IsValid(InteractionTarget) || !IsInteractable(InteractionTarget)) continue;

		// 타겟의 마커 컴포넌트를 가져옵니다.
		USceneComponent* MarkerComponent = InteractionTarget->MarkerTargetComponent;

		// 마커의 회전 값을 가져옵니다.
		FRotator MarkerRotation = MarkerComponent->GetComponentRotation();

		// 마커 오프셋 값을 회전에 맞게 조정하여 타겟 위치를 계산합니다.
		FVector AdjustedOffset = UKismetMathLibrary::GreaterGreater_VectorRotator(InteractionTarget->MarkerOffset, MarkerRotation);
		FVector TargetLocation = MarkerComponent->GetComponentLocation() + AdjustedOffset;

		// 플레이어 카메라 매니저를 가져옵니다.
		TObjectPtr<APlayerCameraManager> PlayerCameraManager = OwnerController->PlayerCameraManager;

		// 타겟 방향 벡터를 계산합니다.
		FVector DirectionToTarget = UKismetMathLibrary::Normal(TargetLocation - PlayerCameraManager->GetCameraLocation());

		// 카메라의 정면 방향 벡터를 가져옵니다.
		FVector CameraForwardDirection = UKismetMathLibrary::GetForwardVector(PlayerCameraManager->GetCameraRotation());

		// 카메라 방향과 타겟 방향의 내적(Dot Product)을 계산합니다.
		float CurrentDotProduct = FVector::DotProduct(DirectionToTarget, CameraForwardDirection);

		// 내적 값이 0.5보다 크고, 지금까지 계산된 내적 값 중 가장 큰 값일 경우
		// 해당 타겟을 최적의 타겟으로 설정합니다.
		if (CurrentDotProduct > 0.5f && CurrentDotProduct > HighestDotProduct)
		{
			HighestDotProduct = CurrentDotProduct;
			BestTarget = InteractionTarget;
		}
	}

	return BestTarget; // 가장 적합한 타겟을 반환합니다.
}

//도마에요
void UInteractionManager::UpdateBestInteractable(UInteractionTarget* NewTarget)
{
	// 새로운 상호작용 대상이 유효하고, 현재 상호작용 대상과 다를 경우 처리
	if (IsValid(NewTarget) && BestInteractionTarget != NewTarget)
	{
		if (IsValid(BestInteractionTarget))
		{
			ClientSetNewTarget(BestInteractionTarget, false);

			if (bIsInteracting)
			{
				// 임시 주석
				// 현재 고개를 돌려서 BestInteractionTarget이 변경되면 Cancel보내는 것 차단.
				//
				// Cancel된 상태에서 
				// bIsInteracting = false;
				//
				// if (BestInteractionTarget->OnInteractionEnd.IsBound())
				// 	BestInteractionTarget->OnInteractionEnd.Broadcast(EInteractionResult::Canceled, OwnerController->GetPawn());

				// Client Reset Data 호출
				if (BestInteractionTarget->OwnerReference->GetClass()->ImplementsInterface(UInterface_Interaction::StaticClass()))
					IInterface_Interaction::Execute_ResetData(BestInteractionTarget->OwnerReference);
			}
		}

		BestInteractionTarget = NewTarget;
		ClientSetNewTarget(BestInteractionTarget, true);
	}
	else if (!IsValid(NewTarget))
	{
		if (IsValid(BestInteractionTarget))
		{
			ClientSetNewTarget(BestInteractionTarget, false);

			if (bIsInteracting)
			{
				// bIsInteracting = false;

				// if (BestInteractionTarget->OnInteractionEnd.IsBound())
				// 	BestInteractionTarget->OnInteractionEnd.Broadcast(EInteractionResult::Canceled, OwnerController->GetPawn());

				if (BestInteractionTarget->GetOwner()->GetClass()->ImplementsInterface(UInterface_Interaction::StaticClass()))
					IInterface_Interaction::Execute_ResetData(BestInteractionTarget->GetOwner());
			}

			BestInteractionTarget = nullptr;
			ClientSetNewTarget(nullptr, false);
		}
	}
}

bool UInteractionManager::IsInteractable(UInteractionTarget* InteractionTarget)
{
	// 상호작용 대상이 활성화되어 있고, 대기 목록이나 비활성화된 목록에 포함되지 않은 경우에만 true를 반환합니다.
	return InteractionTarget->IsInteractionEnabled() && !(PendingTargets.Contains(InteractionTarget) || DeactivatedTargets.Contains(InteractionTarget));
}

bool UInteractionManager::GetInteractionKeys(TArray<FKey>& OutInteractionKeys)
{
	if (IsValid(BestInteractionTarget))
	{
		if (BestInteractionTarget->bUseCustomKeys)
		{
			OutInteractionKeys = BestInteractionTarget->CustomKeys;
			return BestInteractionTarget->CustomKeys.IsValidIndex(0);
		}
	}

	OutInteractionKeys = InteractionKeys;
	return InteractionKeys.IsValidIndex(0);
}

UUW_InteractionTarget* UInteractionManager::FindWidgetByInteractionTarget(UInteractionTarget* InteractionTarget)
{
	for (UUW_InteractionTarget* InteractionWidget : WidgetPool)
	{
		if (InteractionWidget->WidgetInteractionTarget == InteractionTarget)
			return InteractionWidget;
	}
	return nullptr;
}

UUW_InteractionTarget* UInteractionManager::FindEmptyWidget()
{
	for (UUW_InteractionTarget* InteractionWidget : WidgetPool)
	{
		if (InteractionWidget->WidgetInteractionTarget == nullptr)
		{
			return InteractionWidget;
		}
	}
	return nullptr;
}

void UInteractionManager::SetTargetHighlighted(UInteractionTarget* InteractionTarget, bool bIsHighlighted)
{
	Outline_DynamicMaterial->SetVectorParameterValue(FName(TEXT("Outline Color")), InteractionTarget->HighlightColor); // 강조 표시 색상을 InteractionTarget의 HighlightColor로 설정

	InteractionTarget->SetHighlight(bIsHighlighted); // InteractionTarget의 강조 표시 상태를 설정
}

void UInteractionManager::AddToDeactivatedTargets(UInteractionTarget* InteractionTarget)
{
	DeactivatedTargets.AddUnique(InteractionTarget);
	ServerUpdatePointOfInterests(false, InteractionTarget);
	ServerUpdateInteractionTargets(false, InteractionTarget);
	InteractionTarget->OnDeactivated();
}

void UInteractionManager::RemoveFromDeactivatedTargets(UInteractionTarget* InteractionTarget)
{
	DeactivatedTargets.Remove(InteractionTarget);
}

void UInteractionManager::OnNewTargetSelectedClientSide(UInteractionTarget* NewTarget, bool bIsSelected)
{
	// 새로운 상호작용 대상을 설정
	BestInteractionTarget = NewTarget;

	// 새로운 상호작용 대상이 선택되었을 경우
	if (bIsSelected)
	{
		SetTargetHighlighted(BestInteractionTarget, true); // 대상의 하이라이트를 활성화

		CurrentInteractionMarker = FindWidgetByInteractionTarget(BestInteractionTarget); // 상호작용 대상에 연결된 위젯을 찾음

		// 상호작용 키를 가져와서 위젯에 설정
		TArray<FKey> TargetInteractionKeys;
		GetInteractionKeys(TargetInteractionKeys);
		CurrentInteractionMarker->SetInteractionKeyText(TargetInteractionKeys[0]);

		CurrentInteractionMarker->OnWidgetNewTarget(true); // 위젯에 새로운 대상이 선택되었음을 알림

		
		// 추가한 부분 진행도를 유지시켜준다.
		// InteractableActor를 cast로 가져오는데 InteractionTarget에 Owner를 가져오는 함수를 만들까
		float DeltaTime = Cast<AInteractableActor>(BestInteractionTarget->GetOwner())->CurrentHoldTime;
		float NormalizedHoldTime = UKismetMathLibrary::NormalizeToRange(DeltaTime, 0.0f, BestInteractionTarget->HoldSeconds);
		float InteractionRatio = FMath::Min(NormalizedHoldTime, 1.0f);
		
		CurrentInteractionMarker->SetInteractionPercent(InteractionRatio);
		
		// 추가한 부분 끝
	}
	// 상호작용 대상이 선택 해제되었을 경우
	else
	{
		// 현재 선택된 위젯이 유효한 경우 처리
		if (IsValid(CurrentInteractionMarker))
		{
			CurrentInteractionMarker->OnWidgetNewTarget(false); // 위젯에 대상이 선택 해제되었음을 알림
			CurrentInteractionMarker = nullptr; // 현재 상호작용 마커를 nullptr로 설정
			SetTargetHighlighted(BestInteractionTarget, false); // 대상의 하이라이트를 비활성화 
		}
	}
}

void UInteractionManager::OnInteractionTargetDestroyed(AActor* DestroyedActor)
{
	UInteractionTarget* InteractionTarget = DestroyedActor->GetComponentByClass<UInteractionTarget>();
	if (!InteractionTarget) return;

	InteractionTargets.Remove(InteractionTarget);
	PointOfInterests.Remove(InteractionTarget);
	ClientOnInteractionTargetDestroyed(InteractionTarget);
	DestroyedActor->OnDestroyed.RemoveAll(this);
}

void UInteractionManager::ServerRequestAssignInteractor_Implementation(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	InteractionTarget->AssignInteractor(bIsAdd, OwnerController);
}

void UInteractionManager::ClientOnInteractionTargetDestroyed_Implementation(UInteractionTarget* ActorInteractionTarget)
{
	CurrentInteractionMarker->UpdateInteractionTarget(nullptr);
}

void UInteractionManager::ClientUpdatePointOfInterests_Implementation(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	OnPointOfInterestUpdatedClientSide(bIsAdd, InteractionTarget);
}

void UInteractionManager::ServerUpdatePointOfInterests_Implementation(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	OnPointOfInterestUpdatedServerSide(bIsAdd, InteractionTarget);
}

void UInteractionManager::ClientUpdateInteractionTargets_Implementation(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	OnInteractionTargetUpdatedClientSide(true, InteractionTarget);
}

void UInteractionManager::OnInteractionTargetUpdatedServerSide(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	if (bIsAdd)
	{
		if (IsInteractable(InteractionTarget))
		{
			if (InteractionTarget->GetOwner()->GetClass()->ImplementsInterface(UInterface_Interaction::StaticClass()))
			{
				IInterface_Interaction::Execute_SetEnableInteractivity(InteractionTarget->GetOwner(),true);
			}
			ClientUpdateInteractionTargets(true, InteractionTarget);
			InteractionTargets.Add(InteractionTarget);
		}
	}
	else
	{
		if (!InteractionTarget->bIsInteracting)
		{
			if (InteractionTarget->OwnerReference->GetClass()->ImplementsInterface(UInterface_Interaction::StaticClass()))
			{
				IInterface_Interaction::Execute_SetEnableInteractivity(InteractionTarget->OwnerReference, false);
			}
		}
		
		InteractionTargets.Remove(InteractionTarget);
		ClientUpdateInteractionTargets(false, InteractionTarget);
	}
}

void UInteractionManager::OnInteractionTargetUpdatedClientSide(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	// 상호작용 대상과 연결된 위젯을 찾습니다.
	UUW_InteractionTarget* InteractionTargetWidget = FindWidgetByInteractionTarget(InteractionTarget);

	// 위젯이 유효한 경우, 위젯의 콘텐츠 상태를 업데이트합니다.
	if (IsValid(InteractionTargetWidget))
	{
		InteractionTargetWidget->UpdateContentState(bIsAdd);
	}
}

void UInteractionManager::OnPointOfInterestUpdatedServerSide(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	if (!IsValid(InteractionTarget))
		return;

	if (bIsAdd)
	{
		if (IsInteractable(InteractionTarget))
		{
			PointOfInterests.Add(InteractionTarget);
			InteractionTarget->OwnerReference->OnDestroyed.AddDynamic(this, &UInteractionManager::OnInteractionTargetDestroyed);
			ClientUpdatePointOfInterests(true, InteractionTarget);
		}
		else if (InteractionTarget->IsReactivationEnabled())
		{
			InteractionTarget->InteractionFinishExecute(this, EInteractionResult::None);
		}
	}
	else
	{
		PointOfInterests.Remove(InteractionTarget);
		ClientUpdatePointOfInterests(false, InteractionTarget);
	}
}

void UInteractionManager::OnPointOfInterestUpdatedClientSide(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	if (!InteractionTarget)
		return;

	if (bIsAdd)
	{
		// 위젯 정보를 업데이트합니다.
		InteractionTarget->UpdateWidgetInfo(WidgetScreenMargin, ScreenRadiusPercent);

		// 빈 위젯을 찾거나, 새로운 위젯을 생성하여 상호작용 타겟을 설정합니다.
		UUW_InteractionTarget* InteractionWidget = FindEmptyWidget();

		if (!InteractionWidget && OwnerController->IsLocalPlayerController())
		{
			InteractionWidget = CreateWidget<UUW_InteractionTarget>(OwnerController, InteractionTargetWidgetClass);
			WidgetPool.AddUnique(InteractionWidget);
			InteractionWidget->AddToPlayerScreen();
		}

		if (InteractionWidget)
		{
			InteractionWidget->UpdateInteractionTarget(InteractionTarget);
		}
	}
	else
	{
		// 상호작용 타겟과 연결된 위젯을 찾아서 초기화합니다.
		if (UUW_InteractionTarget* AssociatedWidget = FindWidgetByInteractionTarget(InteractionTarget))
		{
			AssociatedWidget->UpdateInteractionTarget(nullptr);
		}
	}
}

void UInteractionManager::ServerUpdateInteractionTargets_Implementation(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	OnInteractionTargetUpdatedServerSide(bIsAdd, InteractionTarget);
}

void UInteractionManager::ClientSetNewTarget_Implementation(UInteractionTarget* NewTarget, bool bIsSelected)
{
	OnNewTargetSelectedClientSide(NewTarget, bIsSelected);
}
