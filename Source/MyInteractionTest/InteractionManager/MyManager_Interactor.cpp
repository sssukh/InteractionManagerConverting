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
	static ConstructorHelpers::FClassFinder<UUserWidget> InteractionWidgetClassRef (TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/InteractionManager/UserInterface/WB_InteractionTarget.WB_InteractionTarget_C'"));
	if(InteractionWidgetClassRef.Class)
	{
		InteractionWidgetClass = InteractionWidgetClassRef.Class;
	}

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
}



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

void UMyManager_Interactor::OnInteractionTargetUpdatedClientSide(bool Add,
	UMyManager_InteractionTarget* InteractionTarget)
{
	UUW_InteractionTarget* InteractionWidget = FindWidgetByInteractionTarget(InteractionTarget);

	if(InteractionWidget)
	{
		 InteractionWidget->UpdateContentState(Add);
	}
}

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

// void UMyManager_Interactor::TickComponent(float DeltaTime, ELevelTick TickType,
// 	FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
// 	Debug_Function();
//
//
// 	// Switch Has Authority in Blueprint
// 	if(GetOwner()->HasAuthority())
// 	{
// 		Find_Best_Interactable();
//
// 		// Update_Best_Interactable();
// 	}
// }

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
	if(!InteractionWidgetClass)
	{
		// ToDo 로그 추가해주세요.
		// !LOG(TEXT("InteractionWidgetClass를 설정하지 않았습니다 꼭 설정해주세요"));
		return;
	}
	
	
	if(OwnerController->IsLocalPlayerController())
	{
		for(int i=0;i<DefaultWidgetPoolSize;++i)
		{
			UUW_InteractionTarget* NewInteractionWidget = CreateWidget<UUW_InteractionTarget>(OwnerController,InteractionWidgetClass);
			
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
		// switch (InteractionTarget->FinishMethod)
		// {
		// 	
		// }
	}
	else
	{
		
	}
}

void UMyManager_Interactor::OnInteractionTargetDestroyed(AActor* DestroyedActor)
{
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


// bool UMyManager_Interactor::GetInteractionKeys(TArray<FKey>& ReturnKeyRef) const
// {
// 	if(BestInteractionTarget&&BestInteractionTarget->UseCustomKeys)
// 	{
// 		ReturnKeyRef = BestInteractionTarget->CustomKeys;
// 		return ReturnKeyRef.Num()>0;
// 	}
// 	
// 	ReturnKeyRef = InteractionKeys;
// 	return InteractionKeys.Num()>0;
// }


// UMyManager_InteractionTarget* UMyManager_Interactor::Find_Best_Interactable()
// {
// 	double LocDotProduct =0.0;
//
// 	UMyManager_InteractionTarget* LocTargetInteractable = nullptr;
// 	for (UMyManager_InteractionTarget* LocCachedInteractable : InteractionTargets)
// 	{
// 		// IsInteractable? 여기있음 IsInteractable로 수정하자.
// 		// PendingTargets와 DeactivatedTargets에 LocCachedInteractable이 없고 멤버변수 InteractionEnabled가 true이면
// 		if(!(PendingTargets.Contains(LocCachedInteractable)||DeactivatedTargets.Contains(LocCachedInteractable))
// 			&&LocCachedInteractable->IsInteractionEnabled())
// 		{
// 			FVector LocActorOrigin = LocCachedInteractable->MarkerTargetComponent->GetComponentLocation()
// 									+ LocCachedInteractable->MarkerTargetComponent->GetComponentRotation().RotateVector(LocCachedInteractable->MarkerOffset);
//
// 			FVector SubResult = LocActorOrigin-OwnerController->PlayerCameraManager->GetCameraLocation();
//
// 			FVector PlayerCameraManagerForwardVector = OwnerController->PlayerCameraManager->GetCameraRotation().Vector();
//
// 			double DotResult = FVector::DotProduct(SubResult.GetSafeNormal(0.0001),PlayerCameraManagerForwardVector);
//
// 			if(DotResult>0.5&&DotResult>LocDotProduct)
// 			{
// 				LocDotProduct = DotResult;
// 				LocTargetInteractable = LocCachedInteractable;
// 			}
// 		}
// 	}
//
// 	return LocTargetInteractable;
// }
//
// void UMyManager_Interactor::Update_Best_Interactable(UMyManager_InteractionTarget* NewTarget)
// {
// 	if(NewTarget)
// 	{
// 		if(NewTarget!=BestInteractionTarget)
// 		{
// 			if(BestInteractionTarget)
// 			{
// 				// ClientSetNewTarget
// 			}
// 		}
// 	}
// 	else
// 	{
// 		
// 	}
// }

// void UMyManager_Interactor::OnNewTargetSelectedClientSide(UMyManager_InteractionTarget* NewTarget, bool IsSelected)
// {
// 	if(NewTarget)
// 	{
// 		BestInteractionTarget = NewTarget;
//
// 		if(IsSelected)
// 		{
// 			SetTargetHighlighted(BestInteractionTarget,true);
//
// 			CurrentInteractionMarker = Find_Widget_By_InteractionTarget(BestInteractionTarget);
//
// 			TArray<FKey> Keys;
// 			GetInteractionKeys(Keys);
//
// 			// SetInteractionKeyText;
// 		}
// 		else
// 		{
// 			
// 		}
// 	}
// }
//
// void UMyManager_Interactor::SetTargetHighlighted(UMyManager_InteractionTarget* InteractionTarget, bool IsHighlighted)
// {
// 	Outline_DynamicMaterial->SetVectorParameterValue(TEXT("OutlineColor"),InteractionTarget->HighlightColor);
//
// 	InteractionTarget->SetHighlight(IsHighlighted);
// }


