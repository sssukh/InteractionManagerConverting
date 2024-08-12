// Fill out your copyright notice in the Description page of Project Settings.


#include "MyManager_Interactor.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PostProcessComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyManager_InteractionTarget.h"
#include "Components/SphereComponent.h"

#include "UserInterface/UW_InteractionTarget.h"


// Sets default values for this component's properties
UMyManager_Interactor::UMyManager_Interactor()
{
	DefaultWidgetPoolSize = 3;
	
	// 클래스 지정을 해줘야함.


	static ConstructorHelpers::FObjectFinder<UMaterialInterface> OutlineMaterialRef(TEXT("/Script/Engine.Material'/Game/InteractionManager/Environment/Materials/PostProcess/M_OutlineMaterial.M_OutlineMaterial'"));
	if(OutlineMaterialRef.Object)
	{
		m_OutlineMaterial = OutlineMaterialRef.Object;
	}

	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UMyManager_Interactor::BeginPlay()
{
	Super::BeginPlay();

	ConstructPlayerEssentials();

	ConstructPooledMarkerWidgets();
	// ...

}

void UMyManager_Interactor::ClientResetData_Implementation()
{
	KeyJustPressed = false;

	LastPressedKey = FKey();

	CurrentHoldTime = 0.0f;

	Repeated = 0;
}

void UMyManager_Interactor::ServerRequestAssignInteractor_Implementation(bool Add,
                                                                         UMyManager_InteractionTarget* InteractionTarget)
{
	if(InteractionTarget)
	{
		InteractionTarget->AssignInteractor(Add,OwnerController);
	}
}

bool UMyManager_Interactor::ServerRequestAssignInteractor_Validate(bool Add,
	UMyManager_InteractionTarget* InteractionTarget)
{
	return true;
}

void UMyManager_Interactor::ServerUpdateInteractionTargets_Implementation(bool Add,
                                                                          UMyManager_InteractionTarget* InteractionTarget)
{
	OnInteractionTargetUpdatedServerSide(Add,InteractionTarget);
}

bool UMyManager_Interactor::ServerUpdateInteractionTargets_Validate(bool Add, UMyManager_InteractionTarget* InteractionTarget)
{
	// 나중에 내용 확인해야함.

	return true;
}

void UMyManager_Interactor::ClientUpdateInteractionTargets_Implementation(bool Add,
	UMyManager_InteractionTarget* InteractionTarget)
{
	OnInteractionTargetUpdatedClientSide(Add, InteractionTarget);
}

void UMyManager_Interactor::ServerUpdatePointOfInterests_Implementation(bool Add,
	UMyManager_InteractionTarget* InteractionTarget)
{
	OnPointOfInterestUpdatedServerSide(Add,InteractionTarget);
}

bool UMyManager_Interactor::ServerUpdatePointOfInterests_Validate(bool Add,
	UMyManager_InteractionTarget* InteractionTarget)
{
	// 나중에 내용 확인해야함.
	return true;
}

void UMyManager_Interactor::ClientUpdatePointOfInterests_Implementation(bool Add,
	UMyManager_InteractionTarget* InteractionTarget)
{
	OnPointOfInterestUpdatedClientSide(Add,InteractionTarget);
}


void UMyManager_Interactor::ClientOnInteractionTargetDestroyed_Implementation(
	UMyManager_InteractionTarget* InteractionTarget)
{
	CurrentInteractionMarker->UpdateInteractionTarget(nullptr);
}

void UMyManager_Interactor::ServerOnInteractionBegin_Implementation(UMyManager_InteractionTarget* InteractionTarget)
{
	InteractionTarget->OnInteractionBegin.Broadcast(OwnerController->GetPawn());

	IsInteracting = true;
}

bool UMyManager_Interactor::ServerOnInteractionBegin_Validate(UMyManager_InteractionTarget* InteractionTarget)
{
	return true;
}

void UMyManager_Interactor::ServerOnInteractionUpdated_Implementation(UMyManager_InteractionTarget* InteractionTarget,
	double Alpha, int32 InRepeated, APawn* InteractorPawn)
{
	InteractionTarget->OnInteractionUpdated.Broadcast(Alpha,InRepeated,InteractorPawn);
}

bool UMyManager_Interactor::ServerOnInteractionUpdated_Validate(UMyManager_InteractionTarget* InteractionTarget,
	double Alpha, int32 InRepeated, APawn* InteractionPawn)
{
	return true;
}

void UMyManager_Interactor::OnInteractionUpdated(UMyManager_InteractionTarget* InteractionTarget, double Alpha,
                                                 int32 InRepeated)
{
	InteractionTarget->OnInteractionUpdated.Broadcast(Alpha,InRepeated,OwnerController->GetPawn());

	// standAlone이 아니면
	if(!(GEngine->GetWorld()->GetNetMode() == NM_Standalone))
	{
		ServerOnInteractionUpdated(InteractionTarget,Alpha,InRepeated,OwnerController->GetPawn());
	}
}

// Add : Begin Overlap -> true, End overlap -> false
// Add InteractionTarget.
void UMyManager_Interactor::OnInteractionTargetUpdatedServerSide(bool Add,
	UMyManager_InteractionTarget* InteractionTarget)
{
	if(Add)
	{
		if(IsInteractable(InteractionTarget))
		{
			ClientUpdateInteractionTargets(true,InteractionTarget);

			InteractionTargets.AddUnique(InteractionTarget);
		}
	}
	else
	{
		InteractionTargets.Remove(InteractionTarget);
		ClientUpdateInteractionTargets(false,InteractionTarget);
	}
}

// Find Widget and Update Content
void UMyManager_Interactor::OnInteractionTargetUpdatedClientSide(bool Add,
	UMyManager_InteractionTarget* InteractionTarget)
{
	UUW_InteractionTarget* InteractionWidget = FindWidgetByInteractionTarget(InteractionTarget);

	if(InteractionWidget)
	{
		 InteractionWidget->UpdateContentState(Add);
	}
}
// Add Point Of Interests
// Add OnDestroy Delegate
// Client Update Point of Interest
// Delete Every InteractionTargets And Widget In Interactor's Interacting Array
void UMyManager_Interactor::OnPointOfInterestUpdatedServerSide(bool Add,
	UMyManager_InteractionTarget* InteractionTarget)
{
	if(Add)
	{
		if(IsInteractable(InteractionTarget))
		{
			PointOfInterests.AddUnique(InteractionTarget);

			InteractionTarget->OwnerReference->OnDestroyed.AddDynamic(this,
				&UMyManager_Interactor::OnInteractionTargetDestroyed);

			ClientUpdatePointOfInterests(true,InteractionTarget);
		}
		else
		{
			if(InteractionTarget->IsReactivationEnabled())
			{
				AddToPendingTargets(InteractionTarget);
			}
		}
	}
	else
	{
		if(InteractionTarget)
		{
			PointOfInterests.Remove(InteractionTarget);

			ClientUpdatePointOfInterests(false,InteractionTarget);
		}
	}
}

void UMyManager_Interactor::OnPointOfInterestUpdatedClientSide(bool Add,
	UMyManager_InteractionTarget* InteractionTarget)
{
	if(Add)
	{
		InteractionTarget->UpdateWidgetInfo(WidgetScreenMargin,ScreenRadiusPercent);

		if(UUW_InteractionTarget* InteractionWidget = FindEmptyWidget())
		{
			InteractionWidget->UpdateInteractionTarget(InteractionTarget);
		}
		else
		{
			// InteractionWidget이 생성되지만 아무 값도 없고 InteractionTarget에 등록도 안됨.
			if(OwnerController->IsLocalController())
			{
				
				InteractionWidget = Cast<UUW_InteractionTarget>(CreateWidget(OwnerController,UUW_InteractionTarget::StaticClass()));

				WidgetPool.AddUnique(InteractionWidget);

				InteractionWidget->AddToPlayerScreen();

				InteractionWidget->UpdateInteractionTarget(InteractionTarget);
			}
		}
	}
	else
	{
		if(UUW_InteractionTarget* InteractionWidget = FindWidgetByInteractionTarget(InteractionTarget))
		{
			// 그래프에 빈칸을 넣어서 nullptr 넣음
			InteractionWidget->UpdateInteractionTarget(nullptr);
		}
	}
	
}

void UMyManager_Interactor::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Debug_Function();


	// Switch Has Authority in Blueprint
	if(GetOwner()->HasAuthority())
	{
		UMyManager_InteractionTarget* NewTarget = FindBestInteractable();

		UpdateBestInteractable(NewTarget);
	}
}

void UMyManager_Interactor::ServerOnInteractionFinished_Implementation(UMyManager_InteractionTarget* InteractionTarget,
	Enum_InteractionResult Result)
{
	ApplyFinishMethod(InteractionTarget,Result);
}

bool UMyManager_Interactor::ServerOnInteractionFinished_Validate(UMyManager_InteractionTarget* InteractionTarget,
	Enum_InteractionResult Result)
{
	return true;
}

void UMyManager_Interactor::ClientCheckPressedKey_Implementation()
{
	TryTakeInteraction();
}

void UMyManager_Interactor::ClientSetNewTarget_Implementation(UMyManager_InteractionTarget* NewTarget, bool IsSelected)
{
	OnNewTargetSelectedClientSide(NewTarget,IsSelected);
}

// Call This Event "On Possess" In The Controller If You Dynamically Change Pawns During Gameplay
// Caches The Controller, Updates The Interaction Keys And Post Process Component
void UMyManager_Interactor::ConstructPlayerEssentials()
{
	APlayerController* CastPlayerController = Cast<APlayerController>(GetOwner());
	if(!CastPlayerController)
	{
		return;
	}
	 
	 OwnerController = CastPlayerController;
	

	ConstructPostProcessComponent();

	// Update_InteractionKeys() 찾기
	GetWorld()->GetTimerManager().SetTimer(BeginUpdateKeysTimerHandle,FTimerDelegate::CreateLambda(
		[&]()
		{
			Update_InteractionKeys();
		}
		), 0.2f,false,0.0f);
}

// Create A Pool For Markers
void UMyManager_Interactor::ConstructPooledMarkerWidgets()
{
	// if(!InteractionWidgetClass)
	// {
	// 	// ToDo 로그 추가해주세요.
	// 	// !LOG(TEXT("InteractionWidgetClass를 설정하지 않았습니다 꼭 설정해주세요"));
	// 	return;
	// }
	
	
	if(OwnerController->IsLocalPlayerController())
	{
		for(int i=0;i<DefaultWidgetPoolSize;++i)
		{
			UUW_InteractionTarget* NewInteractionWidget = CreateWidget<UUW_InteractionTarget>(OwnerController,UUW_InteractionTarget::StaticClass());
			
			WidgetPool.AddUnique(NewInteractionWidget);
			
			NewInteractionWidget->AddToPlayerScreen();
		}
	}
	
	
	
}

// Adds Post Process Component To The Player Pawn And Sets The Outline Materials Of It
void UMyManager_Interactor::ConstructPostProcessComponent()
{
	// AddComponent가 나중에 값이 잘 적용되지 않는다고 한다. NewObject로 수정이 필요하다.
	// PostProcessComponent = Cast<UPostProcessComponent>(OwnerController->GetPawn()->
	// 	AddComponentByClass(UPostProcessComponent::StaticClass(),true,FTransform::Identity,false));
	// OwnerController->GetPawn()->AddInstanceComponent(PostProcessComponent);

	// NewObject로 변경했으니 디버그로 확인하기
	if(PostProcessComponent = NewObject<UPostProcessComponent>(this->GetOwner(),
		UPostProcessComponent::StaticClass(),TEXT("PostProcessComponent")))
	{
		PostProcessComponent->RegisterComponent();
		Outline_DynamicMaterial  = UMaterialInstanceDynamic::Create(m_OutlineMaterial, this );

		FWeightedBlendable TempWeightedBlendable;
		TempWeightedBlendable.Object = Outline_DynamicMaterial;
		TempWeightedBlendable.Weight = 1.0f;
	
		PostProcessComponent->Settings.WeightedBlendables.Array.Add(TempWeightedBlendable);
	}
}

void UMyManager_Interactor::Update_InteractionKeys()
{
	// InteractionKey에 값이 들어오게 로직을 수정했다.
	if(OwnerController->IsLocalPlayerController())
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwnerController->GetLocalPlayer()))
		{
			InteractionKeys = Subsystem->QueryKeysMappedToAction(InteractionInputAction);
			GetWorld()->GetTimerManager().ClearTimer(BeginUpdateKeysTimerHandle);
		}
	}
}




void UMyManager_Interactor::Debug_Function()
{
	if(Debug)
	{
		if(GetOwner()->HasAuthority())
		{
			GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::Blue,FString::Printf(TEXT("Interaction Targets : %d"),InteractionTargets.Num()));
			GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::Blue,FString::Printf(TEXT("Point Of Interests : %d"),PointOfInterests.Num()));
			GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::Blue,FString::Printf(TEXT("Pending Targets : %d"),PendingTargets.Num()));
			if(CurrentInteractionMarker)
				GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::Blue,FString::Printf(TEXT("Marker : %s"),*CurrentInteractionMarker->GetName()));
			
		}
	}
}

UUW_InteractionTarget* UMyManager_Interactor::FindEmptyWidget()
{
	UUW_InteractionTarget* LocReturn = nullptr;
	for (UUW_InteractionTarget* Widget : WidgetPool)
	{
		if(Widget->WidgetInteractionTarget==nullptr)
		{
			LocReturn = Widget;
			break;
		}
	}
	return LocReturn;
}


bool UMyManager_Interactor::IsInteractable(UMyManager_InteractionTarget* ItemToFind)
{
	return (!(PendingTargets.Contains(ItemToFind)||DeactivatedTargets.Contains(ItemToFind))
			&&ItemToFind->IsInteractionEnabled());
}

UUW_InteractionTarget* UMyManager_Interactor::FindWidgetByInteractionTarget(UMyManager_InteractionTarget* InteractionTarget)
{
	UUW_InteractionTarget* LocReturn = nullptr;
	for (UUW_InteractionTarget* Widget : WidgetPool)
	{
		if(Widget->WidgetInteractionTarget == InteractionTarget)
		{
			LocReturn = Widget;
		}
	}

	return LocReturn;
}

void UMyManager_Interactor::ApplyFinishMethod(UMyManager_InteractionTarget* InteractionTarget,
	Enum_InteractionResult Result)
{
	IsInteracting = false;

	ClientResetData();

	if(OwnerController&&OwnerController->GetPawn())
	InteractionTarget->OnInteractionEnd.Broadcast(Result,OwnerController->GetPawn());

	if(Result==Enum_InteractionResult::Completed)
	{
		switch (InteractionTarget->FinishMethod)
		{
		case Enum_InteractionFinishMethod::DestroyOnComplete:
		case Enum_InteractionFinishMethod::DestroyOnCompletedOrCanceled:
		case Enum_InteractionFinishMethod::DeactivateOnCanceledDestroyOnCompleted:
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceledDestroyOnCompleted:
			InteractionTarget->OwnerReference->Destroy();
			break;
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompleted:
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedOrCanceled:
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedDestroyOnCanceled:
		case Enum_InteractionFinishMethod::DeactivateOnCanceledReactivateAfterDurationOnCompleted:
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedDeactivateOnCanceled:
			InteractionTarget->OnAddedToPendingTarget();
			break;

		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceledDeactivateOnCompleted:
		case Enum_InteractionFinishMethod::DeactivateOnCompleted:
		case Enum_InteractionFinishMethod::DeactivateOnCompletedOrCanceled:
		case Enum_InteractionFinishMethod::DeactivateOnCompletedDestroyOnCanceled:
		case Enum_InteractionFinishMethod::DeactivateOnCompletedReactivateAfterDurationOnCanceled:
			AddToDeactivatedTargets(InteractionTarget);
			break;
		default:
			break;
		}
	}
	else
	{
		switch (InteractionTarget->FinishMethod)
		{
		case Enum_InteractionFinishMethod::DestroyOnCanceled:
		case Enum_InteractionFinishMethod::DestroyOnCompletedOrCanceled:
		case Enum_InteractionFinishMethod::DeactivateOnCompletedDestroyOnCanceled:
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedDestroyOnCanceled:
			InteractionTarget->OwnerReference->Destroy();
			break;
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceled:
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedOrCanceled:
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceledDestroyOnCompleted:
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceledDeactivateOnCompleted:
		case Enum_InteractionFinishMethod::DeactivateOnCompletedReactivateAfterDurationOnCanceled:
			InteractionTarget->OnAddedToPendingTarget();
			break;
		case Enum_InteractionFinishMethod::DeactivateOnCanceled:
		case Enum_InteractionFinishMethod::DeactivateOnCompletedOrCanceled:
		case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedDeactivateOnCanceled:
		case Enum_InteractionFinishMethod::DeactivateOnCanceledDestroyOnCompleted:
		case Enum_InteractionFinishMethod::DeactivateOnCanceledReactivateAfterDurationOnCompleted:
			AddToDeactivatedTargets(InteractionTarget);
			break;
		default:
			break;
		}
	}
}

void UMyManager_Interactor::OnInteractionTargetDestroyed(AActor* DestroyedActor)
{
	UMyManager_InteractionTarget* LocTarget = nullptr;

	LocTarget = Cast<UMyManager_InteractionTarget>(DestroyedActor->GetComponentByClass(UMyManager_InteractionTarget::StaticClass()));

	InteractionTargets.Remove(LocTarget);

	PointOfInterests.Remove(LocTarget);

	ClientOnInteractionTargetDestroyed(LocTarget);
}

void UMyManager_Interactor::AddToPendingTargets(UMyManager_InteractionTarget* InteractionTarget)
{
	PendingTargets.AddUnique(InteractionTarget);

	ServerUpdatePointOfInterests(false,InteractionTarget);

	ServerUpdateInteractionTargets(false,InteractionTarget);

	if(PendingTargetTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(PendingTargetTimerHandle,
			FTimerDelegate::CreateLambda(
				[&]()
				{
					CheckForPendingTargets();
				}
				),PendingTargetCheckInteraval,true);
	}
}

void UMyManager_Interactor::OnInteractionTargetReactivated(UMyManager_InteractionTarget* InteractionTarget)
{
	PendingTargets.Remove(InteractionTarget);

	InteractionTarget->InteractionEnabled=true;

	if(InteractionTarget->AssignedInteractors.Contains(OwnerController))
	{
		TArray<UPrimitiveComponent*> LocPawnOverlappingComponents;
		
		OwnerController->GetPawn()->GetOverlappingComponents(LocPawnOverlappingComponents);
	
		if(LocPawnOverlappingComponents.Contains(Cast<UPrimitiveComponent>(InteractionTarget->InnerZone)))
		{
			ServerUpdatePointOfInterests(true,InteractionTarget);
	
			ServerUpdateInteractionTargets(true,InteractionTarget);
		}
		else
		{
			if(LocPawnOverlappingComponents.Contains(InteractionTarget->OuterZone))
			{
				ServerUpdatePointOfInterests(true,InteractionTarget);
			}
		}
	}

	InteractionTarget->OnInteractionReactivated.Broadcast(OwnerController->GetPawn());
}

void UMyManager_Interactor::AddToDeactivatedTargets(UMyManager_InteractionTarget* InteractionTarget)
{
	DeactivatedTargets.AddUnique(InteractionTarget);

	ServerUpdatePointOfInterests(false,InteractionTarget);

	ServerUpdateInteractionTargets(false,InteractionTarget);

	InteractionTarget->OnDeactivated();
}

void UMyManager_Interactor::CheckForPendingTargets()
{
	UMyManager_InteractionTarget* LocCurrentPendingTarget=nullptr;
	if(!PendingTargets.IsEmpty())
	{
		for (UMyManager_InteractionTarget* Target : PendingTargets)
		{
			LocCurrentPendingTarget = Target;
			
			if(abs(GEngine->GetWorld()->GetTimeSeconds()-LocCurrentPendingTarget->LastInteractedTime)
				>=LocCurrentPendingTarget->ReactivationDuration)
			{
				OnInteractionTargetReactivated(LocCurrentPendingTarget);
			}
		}
	}
}

void UMyManager_Interactor::RemoveFromDeactivatedTargets(UMyManager_InteractionTarget* InteractionTarget)
{
	DeactivatedTargets.Remove(InteractionTarget);
}


bool UMyManager_Interactor::GetInteractionKeys(TArray<FKey>& ReturnKeyRef) const
{
	if(BestInteractionTarget&&BestInteractionTarget->UseCustomKeys)
	{
		ReturnKeyRef = BestInteractionTarget->CustomKeys;
		return ReturnKeyRef.Num()>0;
	}
	
	ReturnKeyRef = InteractionKeys;
	return InteractionKeys.Num()>0;
}


UMyManager_InteractionTarget* UMyManager_Interactor::FindBestInteractable()
{
	double LocDotProduct =0.0;

	UMyManager_InteractionTarget* LocTargetInteractable = nullptr;
	// InnerZone에 BeginOverlap할 때 InteractionTargets에 InteractionTarget이 추가된다.
	for (UMyManager_InteractionTarget* LocCachedInteractable : InteractionTargets)
	{
		// IsInteractable? 여기있음 IsInteractable로 수정하자.
		// PendingTargets와 DeactivatedTargets에 LocCachedInteractable이 없고 멤버변수 InteractionEnabled가 true이면
		if(LocCachedInteractable && IsInteractable(LocCachedInteractable))
		{
			// MarkerTargetComponent(마커 위치)와 MarkerOffset을 MarkerTargetComponent의 Rotation으로 회전시킨 값을 더한 값
			FVector LocActorOrigin = LocCachedInteractable->MarkerTargetComponent->GetComponentLocation()
									+ LocCachedInteractable->MarkerTargetComponent->GetComponentRotation().RotateVector(LocCachedInteractable->MarkerOffset);

			// 카메라위치에서 ActorOrigin을 가리키는 방향
			FVector SubResult = LocActorOrigin-OwnerController->PlayerCameraManager->GetCameraLocation();

			// 카메라가 바라보는 방향
			FVector PlayerCameraManagerForwardVector = OwnerController->PlayerCameraManager->GetCameraRotation().Vector();

			// 위의 두 방향을 dot product 즉, 내적시킨다.
			// 결과값이 클수록 각이 작고 0이면 직각, 음수면 둔각이다. 
			double DotResult = FVector::DotProduct(SubResult.GetSafeNormal(0.0001),PlayerCameraManagerForwardVector);

			// 즉 카메라 방향과 제일 비슷하면서 방향이 너무 틀어지지 않은 것을 LoctargetInteractable로 삼는다.(라인 트레이스 대신인듯)
			if(DotResult>0.5&&DotResult>LocDotProduct)
			{
				LocDotProduct = DotResult;
				LocTargetInteractable = LocCachedInteractable;
			}
		}
	}

	return LocTargetInteractable;
}

// 가장 플레이어 시점방향과 가까운 타겟을 NewTarget으로 받아온다.
void UMyManager_Interactor::UpdateBestInteractable(UMyManager_InteractionTarget* NewTarget)
{
	if(NewTarget)
	{
		// 받아온 newTarget이 기존 BestInteractionTarget과 동일하면 넘어간다.
		if(NewTarget!=BestInteractionTarget)
		{
			// 기존 BestInteractionTarget과 다르면 해제시키고
			// 현재 Interacting중이면 Interacting 종료하고 상호작용 중이던 값들 전부 초기화
			if(BestInteractionTarget)
			{
				ClientSetNewTarget(BestInteractionTarget,false);

				if(IsInteracting)
				{
					IsInteracting = false;

					BestInteractionTarget->OnInteractionEnd.Broadcast(Enum_InteractionResult::Canceled,
						OwnerController->GetPawn());

					ClientResetData();
				}
			}

			// NewTarget으로 설정 후 Highlight, 위젯 찾아서 CurrentInteractionMarker에 캐싱, 타겟 설정을 해준다.
			BestInteractionTarget = NewTarget;

			ClientSetNewTarget(BestInteractionTarget,true);
		}
	}
	else
	{
		// NewTarget이 없다는 말은 상호작용 위젯을 띄울 최적의 타겟이 존재하지 않는다는 말이기 때문에 모조리 해제시킨다.
		if(BestInteractionTarget)
		{
			ClientSetNewTarget(BestInteractionTarget,false);

			if(IsInteracting)
			{
				IsInteracting = false;

				BestInteractionTarget->OnInteractionEnd.Broadcast(Enum_InteractionResult::Canceled,
					OwnerController->GetPawn());

				ClientResetData();
			}

			BestInteractionTarget = nullptr;
			ClientSetNewTarget(nullptr,false);
		}
	}

	if(BestInteractionTarget)
	{
		ClientCheckPressedKey();
	}
}

void UMyManager_Interactor::TryTakeInteraction()
{
	TArray<FKey> InteractionKeyRef;
	if(GetInteractionKeys(InteractionKeyRef))
	{
		for (FKey KeyRef : InteractionKeyRef)
		{
			if(OwnerController&&OwnerController->WasInputKeyJustPressed(KeyRef))
			{
				LastPressedKey = KeyRef;
				break;
			}
		}

		switch(BestInteractionTarget->InteractionType)
		{
		case Enum_InteractionType::Tap:
			if(KeyJustPressed)
			{
				ServerOnInteractionBegin(BestInteractionTarget);

				CurrentInteractionMarker->UpdateInteractionText(true,Enum_InteractionState::Done);

				CurrentInteractionMarker->PlayInteractionCompletedAnimation(Enum_InteractionResult::Completed);

				ServerOnInteractionFinished(BestInteractionTarget,Enum_InteractionResult::Completed);
			}
			break;
		case Enum_InteractionType::Hold:
			if(OwnerController->IsInputKeyDown(LastPressedKey)&&KeyJustPressed)
			{
				if(CurrentHoldTime==0.0f)
				{
					CurrentInteractionMarker->UpdateInteractionText(false,Enum_InteractionState::Interacting);

					ServerOnInteractionBegin(BestInteractionTarget);
				}
				CurrentHoldTime = GEngine->GetWorld()->GetDeltaSeconds()+CurrentHoldTime;

				double LocValue = FMath::Min(1.0f,CurrentHoldTime/BestInteractionTarget->HoldSeconds);

				CurrentInteractionMarker->SetInteractionPercent(LocValue);

				OnInteractionUpdated(BestInteractionTarget,LocValue,0);

				if(LocValue==1.0f)
				{
					CurrentHoldTime=0.0f;

					CurrentInteractionMarker->PlayInteractionCompletedAnimation(Enum_InteractionResult::Completed);

					ServerOnInteractionFinished(BestInteractionTarget,Enum_InteractionResult::Completed);
				}
			}
			else
			{
				if(CurrentHoldTime!=0.0f)
				{
					if(BestInteractionTarget->CooldownEnabled)
					{
						CurrentHoldTime = FMath::Max(0.0f,CurrentHoldTime - GEngine->GetWorld()->GetDeltaSeconds());

						double LocValue = FMath::Max(0.0f,CurrentHoldTime/BestInteractionTarget->HoldSeconds);
						
						CurrentInteractionMarker->SetInteractionPercent(LocValue);

						if(CurrentHoldTime==0.0f)
						{
							CurrentInteractionMarker->PlayInteractionCompletedAnimation(Enum_InteractionResult::Canceled);

							ServerOnInteractionFinished(BestInteractionTarget,Enum_InteractionResult::Canceled);
						}
						else
						{
							OnInteractionUpdated(BestInteractionTarget,LocValue,0);

							if(BestInteractionTarget->CancelOnRelease())
							{
								ServerOnInteractionFinished(BestInteractionTarget,Enum_InteractionResult::Canceled);
							}
						}
					}
					else
					{
						CurrentHoldTime=0.0f;
						
						CurrentInteractionMarker->SetInteractionPercent(0.0f);

						CurrentInteractionMarker->PlayInteractionCompletedAnimation(Enum_InteractionResult::Canceled);

						ServerOnInteractionFinished(BestInteractionTarget, Enum_InteractionResult::Canceled);
					}
				}
			}
			break;
		case Enum_InteractionType::Repeat:
			if(OwnerController->WasInputKeyJustPressed(LastPressedKey))
			{
				RepeatCooldown = BestInteractionTarget->RepeatCooldown;

				if(Repeated==0)
				{
					CurrentInteractionMarker->UpdateInteractionText(false,Enum_InteractionState::Interacting);

					ServerOnInteractionBegin(BestInteractionTarget);
				}

				Repeated++;

				double LocValue = FMath::Min(1.0f,Repeated/BestInteractionTarget->RepeatCount);

				OnInteractionUpdated(BestInteractionTarget,LocValue,Repeated);

				CurrentInteractionMarker->SetInteractionPercent(LocValue);

				if(Repeated==BestInteractionTarget->RepeatCount)
				{
					Repeated=0;

					CurrentInteractionMarker->PlayInteractionCompletedAnimation(Enum_InteractionResult::Completed);

					ServerOnInteractionFinished(BestInteractionTarget,Enum_InteractionResult::Completed);
				}
				else
				{
					CurrentInteractionMarker->PlayInteractionUpdateAnimation();
				}
			}

			if(BestInteractionTarget)
			{
				if(Repeated!=0 && BestInteractionTarget->CooldownEnabled)
				{
					RepeatCooldown = FMath::Max(RepeatCooldown - GEngine->GetWorld()->GetDeltaSeconds(),0.0f);

					if(RepeatCooldown==0.0f)
					{
						CurrentInteractionMarker->PlayInteractionCompletedAnimation(Enum_InteractionResult::Canceled);

						Repeated--;

						if(Repeated==0)
						{
							CurrentInteractionMarker->SetInteractionPercent(0.0f);

							CurrentInteractionMarker->PlayInteractionCompletedAnimation(Enum_InteractionResult::Canceled);

							ServerOnInteractionFinished(BestInteractionTarget,Enum_InteractionResult::Canceled);

							CurrentInteractionMarker->ResetProgress();

							CurrentInteractionMarker->UpdateInteractionText(false,Enum_InteractionState::Waiting);

							OnInteractionUpdated(BestInteractionTarget,0.0f,Repeated);
						}
					}
					else
					{
						double LocValue = FMath::Min(Repeated/BestInteractionTarget->RepeatCount,1.0f);

						LocValue = CurrentInteractionMarker->CurrentPercent - (GEngine->GetWorld()->GetDeltaSeconds())*(1/((BestInteractionTarget->RepeatCooldown*Repeated)/LocValue));

						CurrentInteractionMarker->SetInteractionPercent(LocValue);
					}
				}
			}
			break;
		default:
			break;
		}
	}

	
}

void UMyManager_Interactor::OnNewTargetSelectedClientSide(UMyManager_InteractionTarget* NewTarget, bool IsSelected)
{
	if(NewTarget)
	{
		BestInteractionTarget = NewTarget;

		if(IsSelected)
		{
			SetTargetHighlighted(BestInteractionTarget,true);

			CurrentInteractionMarker = FindWidgetByInteractionTarget(BestInteractionTarget);

			TArray<FKey> Keys;
			GetInteractionKeys(Keys);

			 CurrentInteractionMarker->SetInteractionKeyText(Keys[0]);

			CurrentInteractionMarker->OnWidgetNewTarget(true);
		}
		else
		{
			if(CurrentInteractionMarker)
			{
				CurrentInteractionMarker->OnWidgetNewTarget(false);

				CurrentInteractionMarker=nullptr;

				SetTargetHighlighted(BestInteractionTarget,false);
			}
		}
	}
}

void UMyManager_Interactor::SetTargetHighlighted(UMyManager_InteractionTarget* InteractionTarget, bool IsHighlighted)
{
	Outline_DynamicMaterial->SetVectorParameterValue(TEXT("OutlineColor"),InteractionTarget->HighlightColor);

	InteractionTarget->SetHighlight(IsHighlighted);
}


