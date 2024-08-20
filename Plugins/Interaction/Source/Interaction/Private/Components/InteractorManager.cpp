#include "Components/InteractorManager.h"

#include "InteractionLog.h"

#include "EnhancedInputSubsystems.h"
#include "InteractionGameplayTags.h"
#include "Actors/InteractableActor.h"
#include "Components/InteractionTarget.h"
#include "Components/PostProcessComponent.h"
#include "Components/SphereComponent.h"
#include "Interfaces/Interface_Interaction.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Objects/InteractionFinish.h"

#include "UserInterface/UW_InteractionTarget.h"


// Sets default values for this component's properties
UInteractionManager::UInteractionManager(): OwnerController(nullptr), PostProcessComponent(nullptr), bIsGamepad(false), bIsInteracting(false), bKeyJustPressed(false), RepeatCooldown(0),
                                            CurrentHoldTime(0), Repeated(0),
                                            CurrentInteractionMarker(nullptr),
                                            Outline_DynamicMaterial(nullptr),
                                            BestInteractionTarget(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	// ...

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

	// ...
	ConstructPlayerEssentials();

	ConstructPooledMarkerWidgets();
}


void UInteractionManager::ConstructPlayerEssentials()
{
	// 현재 오너(Actor)가 APlayerController인지 확인하고 캐스팅합니다.
	APlayerController* CastController = Cast<APlayerController>(GetOwner());

	// 캐스팅에 실패한 경우, 경고 메시지를 출력하고 함수 실행을 중지합니다.
	if (!CastController)
	{
		LOG_WARNING_AND_SCREEN(5.0f, TEXT("APlayerController로 캐스팅에 실패했습니다"))
		return;
	}

	// 캐스팅에 성공한 경우, 소유자 컨트롤러를 저장합니다.
	OwnerController = CastController;

	// 포스트 프로세스 컴포넌트를 생성합니다.
	ConstructPostProcessComponent();

	// 상호 작용 키를 주기적으로 업데이트하기 위한 타이머를 설정합니다.
	// 타이머는 0.2초 간격으로 UpdateInteractionKeys 함수를 호출합니다.
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

	// DefaultWidgetPoolSize에 설정된 크기만큼 위젯 풀을 생성합니다.
	for (int i = 0; i < DefaultWidgetPoolSize; ++i)
	{
		UUW_InteractionTarget* NewInteractionWidget = CreateWidget<UUW_InteractionTarget>(OwnerController, InteractionTargetWidgetClass); // InteractionTargetWidgetClass를 사용하여 새로운 상호작용 타겟 위젯을 생성합니다.
		WidgetPool.AddUnique(NewInteractionWidget); // 생성된 위젯을 WidgetPool 배열에 추가합니다.
		NewInteractionWidget->AddToPlayerScreen(); // 생성된 위젯을 화면에 추가하여 플레이어가 볼 수 있도록 합니다.
	}
}

void UInteractionManager::ConstructPostProcessComponent()
{
	APawn* OwnerPawn = OwnerController->GetPawn();

	// UPostProcessComponent를 OwnerPawn에 추가합니다.
	PostProcessComponent = NewObject<UPostProcessComponent>(OwnerPawn, UPostProcessComponent::StaticClass(), TEXT("PostProcessComponent"));
	PostProcessComponent->RegisterComponent();
	OwnerPawn->AddInstanceComponent(PostProcessComponent);

	// CurrentMaterial을 기반으로 동적 머티리얼 인스턴스를 생성합니다.
	Outline_DynamicMaterial = UMaterialInstanceDynamic::Create(CurrentMaterial, this);

	// 포스트 프로세스 설정에서 WeightedBlendables 배열에 생성한 동적 머티리얼 인스턴스를 추가합니다.
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

	if (GetOwner()->HasAuthority())
	{
		UInteractionTarget* NewTarget = FindBestInteractable();

		UpdateBestInteractable(NewTarget);

		if (IsValid(BestInteractionTarget))
			ClientCheckPressedKey();
	}

	if (bDebug)
		DrawDebugInteractor();
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


void UInteractionManager::UpdateBestInteractable(UInteractionTarget* NewTarget)
{
	// 새로운 상호작용 대상이 유효하고, 현재 상호작용 대상과 다를 경우 처리
	if (IsValid(NewTarget) && BestInteractionTarget != NewTarget)
	{
		// 기존 상호작용 대상이 유효한 경우, 선택 해제 처리
		if (IsValid(BestInteractionTarget))
		{
			ClientSetNewTarget(BestInteractionTarget, false);

			// 상호작용 중인 경우, 상호작용을 중단하고 종료 이벤트를 호출
			if (bIsInteracting)
			{
				bIsInteracting = false;


				if (NewTarget->GetOwner()->GetClass()->ImplementsInterface(UInterface_Interaction::StaticClass()))
				{
					FStateTreeEvent SendEvent;
					SendEvent.Tag = InteractionGameTags::Interaction_End;
					IInterface_Interaction::Execute_SendEvent(NewTarget->GetOwner(), SendEvent);
				}


				ClientResetData();
			}
		}

		BestInteractionTarget = NewTarget;
		ClientSetNewTarget(BestInteractionTarget, true);
	}
	else if (!IsValid(NewTarget)) // 새로운 상호작용 대상이 유효하지 않은 경우
	{
		if (IsValid(BestInteractionTarget))
		{
			ClientSetNewTarget(BestInteractionTarget, false); // 기존 대상의 선택을 해제

			// 상호작용 중인 경우, 상호작용을 중단하고 종료 이벤트를 호출
			if (bIsInteracting)
			{
				bIsInteracting = false;

				if (NewTarget->GetOwner()->GetClass()->ImplementsInterface(UInterface_Interaction::StaticClass()))
				{
					FStateTreeEvent SendEvent;
					SendEvent.Tag = InteractionGameTags::Interaction_End;
					IInterface_Interaction::Execute_SendEvent(NewTarget->GetOwner(), SendEvent);
				}

				ClientResetData();
			}

			BestInteractionTarget = nullptr;
			ClientSetNewTarget(nullptr, false);
		}
	}
}

void UInteractionManager::ApplyFinishMethod(UInteractionTarget* InteractionTarget, EInteractionResult InteractionResult)
{
	bIsInteracting = false;

	ClientResetData();
	
	if (InteractionTarget->GetOwner()->GetClass()->ImplementsInterface(UInterface_Interaction::StaticClass()))
	{
		FStateTreeEvent SendEvent;
		SendEvent.Tag = InteractionGameTags::Interaction_End;
		IInterface_Interaction::Execute_SendEvent(InteractionTarget->GetOwner(), SendEvent);
	}


	if (IsValid(InteractionTarget->InteractionFinishInstance))
	{
		InteractionTarget->InteractionFinishInstance->InitializeOnFinish(this, InteractionTarget);
		InteractionTarget->InteractionFinishInstance->Execute(InteractionResult);
	}
}

void UInteractionManager::ReceiveAnyKey(FKey InKey)
{
	bIsGamepad = InKey.IsGamepadKey();
}

bool UInteractionManager::IsInteractable(UInteractionTarget* ItemToFind)
{
	// 상호작용 대상이 활성화되어 있고, 대기 목록이나 비활성화된 목록에 포함되지 않은 경우에만 true를 반환합니다.
	return ItemToFind->IsInteractionEnabled() && !(PendingTargets.Contains(ItemToFind) || DeactivatedTargets.Contains(ItemToFind));
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
	// 이것은 OnActorDestroyed에 대한 바인딩된 이벤트입니다. 대상이 파괴될 때마다 대상에서 제거되는지 확인합니다.
	UInteractionTarget* ActorInteractionTarget = DestroyedActor->GetComponentByClass<UInteractionTarget>();
	if (!ActorInteractionTarget)
		return;

	InteractionTargets.Remove(ActorInteractionTarget);
	PointOfInterests.Remove(ActorInteractionTarget);
	ClientOnInteractionTargetDestroyed(ActorInteractionTarget);
	DestroyedActor->OnDestroyed.RemoveAll(this);
}

void UInteractionManager::ServerOnInteractionUpdated_Implementation(UInteractionTarget* InteractionTarget, float InAlpha, int32 InRepeated, APawn* InteractorPawn)
{
	if (InteractionTarget->GetOwner()->GetClass()->ImplementsInterface(UInterface_Interaction::StaticClass()))
	{
		FStateTreeEvent SendEvent;
		SendEvent.Tag = InteractionGameTags::Interaction_Update;
		// SendEvent.Payload = FInteractionPayLoad(InAlpha,InRepeated,InteractorPawn);
		IInterface_Interaction::Execute_SendEvent(InteractionTarget->GetOwner(), SendEvent);
	}
}

void UInteractionManager::ServerOnInteractionFinished_Implementation(UInteractionTarget* InteractionTarget, EInteractionResult InteractionResult)
{
	ApplyFinishMethod(InteractionTarget, InteractionResult);
}

void UInteractionManager::ServerOnInteractionBegin_Implementation(UInteractionTarget* InteractionTarget)
{
	if (InteractionTarget->GetOwner()->GetClass()->ImplementsInterface(UInterface_Interaction::StaticClass()))
	{
		FStateTreeEvent SendEvent;
		SendEvent.Tag = InteractionGameTags::Interaction_Begin;
		IInterface_Interaction::Execute_SendEvent(InteractionTarget->GetOwner(), SendEvent);
	}

	bIsInteracting = true;
}

void UInteractionManager::ClientCheckPressedKey_Implementation()
{
	TArray<FKey> TakeInteractionKeys;
	if (!GetInteractionKeys(TakeInteractionKeys))
	{
		LOG_WARNING_AND_SCREEN(5.0f, TEXT("상호작용 키가 존재하지 않습니다"));
		return;
	}

	for (FKey& Key : TakeInteractionKeys)
	{
		if (OwnerController->WasInputKeyJustPressed(Key))
		{
			LastPressedKey = Key;
			break;
		}
	}

	if (AInteractableActor* InteractableActor = Cast<AInteractableActor>(BestInteractionTarget->GetOwner()))
		InteractableActor->TryTakeAction(this);

	if (OwnerController->WasInputKeyJustPressed(LastPressedKey))
		bKeyJustPressed = true;

	if (OwnerController->WasInputKeyJustReleased(LastPressedKey))
		bKeyJustPressed = false;
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
		// 상호작용 타겟이 상호작용 가능한 상태인지 확인합니다.
		if (IsInteractable(InteractionTarget))
		{
			// 클라이언트에게 상호작용 타겟이 추가되었음을 알립니다.
			ClientUpdateInteractionTargets(true, InteractionTarget);
			// 상호작용 타겟 리스트에 새로운 타겟을 추가합니다.
			InteractionTargets.Add(InteractionTarget);
		}
	}
	else
	{
		// 상호작용 타겟 리스트에서 해당 타겟을 제거합니다.
		InteractionTargets.Remove(InteractionTarget);
		// 클라이언트에게 상호작용 타겟이 제거되었음을 알립니다.
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
	if (bIsAdd)
	{
		// 상호작용 대상이 유효하고, 상호작용이 가능할 때 처리
		if (IsInteractable(InteractionTarget))
		{
			// 관심 지점 목록에 상호작용 대상을 추가
			PointOfInterests.Add(InteractionTarget);
			// 상호작용 대상이 소멸될 때 호출될 델리게이트에 바인딩
			InteractionTarget->OwnerReference->OnDestroyed.AddDynamic(this, &UInteractionManager::OnInteractionTargetDestroyed);
			// 클라이언트 측 관심 지점 목록을 업데이트
			ClientUpdatePointOfInterests(true, InteractionTarget);
		}
		else
		{
			// 상호작용 대상이 재활성화 가능하다면, 대기 목록에 추가
			if (InteractionTarget->IsReactivationEnabled())
			{
				InteractionTarget->InteractionFinishInstance->InitializeOnFinish(this, InteractionTarget);
				InteractionTarget->InteractionFinishInstance->Execute(EInteractionResult::None);
			}
		}
	}
	else
	{
		// 상호작용 대상이 유효한 경우에만 처리
		if (IsValid(InteractionTarget))
		{
			// 관심 지점 목록에서 상호작용 대상을 제거
			PointOfInterests.Remove(InteractionTarget);
			// 클라이언트 측 관심 지점 목록을 업데이트
			ClientUpdatePointOfInterests(false, InteractionTarget);
		}
	}
}

void UInteractionManager::OnPointOfInterestUpdatedClientSide(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	if (bIsAdd)
	{
		// POI가 추가될 경우 위젯 정보를 업데이트합니다.
		InteractionTarget->UpdateWidgetInfo(WidgetScreenMargin, ScreenRadiusPercent);
		UUW_InteractionTarget* FindWidget = FindEmptyWidget();

		// 비어 있는 위젯이 있는지 확인하여 재사용합니다.
		if (IsValid(FindWidget))
		{
			FindWidget->UpdateInteractionTarget(InteractionTarget);
		}
		//위젯이 존재하지 않는다면 위젯을 생성합니다.
		else
		{
			// 로컬 플레이어 컨트롤러에서 새 위젯을 생성하여 POI를 표시합니다.
			if (OwnerController->IsLocalPlayerController())
			{
				UUW_InteractionTarget* NewInteractionWidget = CreateWidget<UUW_InteractionTarget>(OwnerController, InteractionTargetWidgetClass);
				WidgetPool.AddUnique(NewInteractionWidget);
				NewInteractionWidget->AddToPlayerScreen();
				NewInteractionWidget->UpdateInteractionTarget(InteractionTarget);
			}
		}
	}
	else
	{
		// POI가 제거될 경우, 관련 위젯을 초기화합니다.
		UUW_InteractionTarget* FindWidget = FindWidgetByInteractionTarget(InteractionTarget);
		if (IsValid(FindWidget))
		{
			FindWidget->UpdateInteractionTarget(nullptr);
		}
	}
}

void UInteractionManager::OnInteractionUpdated(UInteractionTarget* InteractionTarget, float InAlpha, int32 InRepeated)
{
	if (InteractionTarget->GetOwner()->GetClass()->ImplementsInterface(UInterface_Interaction::StaticClass()))
	{
		FStateTreeEvent SendEvent;
		SendEvent.Tag = InteractionGameTags::Interaction_Update;
		IInterface_Interaction::Execute_SendEvent(InteractionTarget->GetOwner(), SendEvent);
	}

	// 현재 환경이 스탠드얼론(싱글플레이어) 모드가 아닌 경우, 서버와 상호작용 상태를 동기화합니다.
	if (!UKismetSystemLibrary::IsStandalone(this))
	{
		ServerOnInteractionUpdated(InteractionTarget, InAlpha, InRepeated, OwnerController->GetPawn());
	}
}

bool UInteractionManager::IsHoldingKey()
{
	return OwnerController->IsInputKeyDown(LastPressedKey) && bKeyJustPressed;
}

void UInteractionManager::ServerUpdateInteractionTargets_Implementation(bool bIsAdd, UInteractionTarget* InteractionTarget)
{
	OnInteractionTargetUpdatedServerSide(bIsAdd, InteractionTarget);
}

void UInteractionManager::ClientResetData_Implementation()
{
	bKeyJustPressed = false;
	LastPressedKey = FKey();
	CurrentHoldTime = 0.0f;
	Repeated = 0;
}

void UInteractionManager::ClientSetNewTarget_Implementation(UInteractionTarget* NewTarget, bool bIsSelected)
{
	OnNewTargetSelectedClientSide(NewTarget, bIsSelected);
}
