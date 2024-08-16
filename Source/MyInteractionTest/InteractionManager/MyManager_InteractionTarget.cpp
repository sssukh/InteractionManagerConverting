// Fill out your copyright notice in the Description page of Project Settings.


#include "MyManager_InteractionTarget.h"

#include "MyManager_Interactor.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UMyManager_InteractionTarget::UMyManager_InteractionTarget()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	InnerZoneRadius = 200.0f;

	OuterZoneExtent = 200.0f;
	// ...
}


// Called when the game starts
void UMyManager_InteractionTarget::BeginPlay()
{
	Super::BeginPlay();

	// Setting Owner
	ConstructOwnerEssentials();

	// Create InnerZone And OuterZone , Set Register
	ConstructOverlapZones();

	// Find Components to Highlight from Owner's Child Components
	ConstructHighlightedComponents();

	// Find MarkerTarget from Owner's ChildComponents.
	// If not, Set Owner's RootComponent
	SelectMarkerComponent(MarkerComponentName);

	OnInteractionBegin.AddDynamic(this, &UMyManager_InteractionTarget::OnInteractionBeginEvent);
	OnInteractionEnd.AddDynamic(this, &UMyManager_InteractionTarget::OnInteractionEndEvent);
}

UMyManager_Interactor* UMyManager_InteractionTarget::GetInteractorManager(AController* Controller)
{
	UMyManager_Interactor* InteractorManager = Cast<UMyManager_Interactor>(Controller->
		GetComponentByClass(UMyManager_Interactor::StaticClass()));

	return InteractorManager;
}

void UMyManager_InteractionTarget::ConstructOwnerEssentials()
{
	if (GetOwner())
	{
		OwnerReference = GetOwner();
	}
}

// InnerZone과 OuterZone 생성 및 Target에 부착
void UMyManager_InteractionTarget::ConstructOverlapZones()
{
	USceneComponent* LocComponentToAttach = nullptr;

	TArray<USceneComponent*> SceneArray;
	GetOwner()->GetComponents(USceneComponent::StaticClass(), SceneArray);

	// InnerZone과 OuterZone을 부착할 Component를 이름으로 찾기
	for (USceneComponent* Scene : SceneArray)
	{
		if (Scene->GetName() == InteractionZone_ComponentToAttach)
		{
			LocComponentToAttach = Scene;
			break;
		}
	}

	// InnerZone 생성해서 위에서 찾은 Component에 부착
	// InnerZone = Cast<USphereComponent>(GetOwner()->
	// 	AddComponentByClass(USphereComponent::StaticClass(),false,FTransform::Identity,false));

	// AddComponentByClass에서 NewObject로 변경함
	if (InnerZone = NewObject<USphereComponent>(this->GetOwner()
	                                            , USphereComponent::StaticClass(),TEXT("InnerZone")))
	{
		InnerZone->RegisterComponent();

		InnerZone->SetHiddenInGame(!EnableDebug);

		if (LocComponentToAttach && InnerZone)
		{
			FAttachmentTransformRules AttachTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld,
			                                               EAttachmentRule::KeepWorld, false);

			InnerZone->AttachToComponent(LocComponentToAttach, AttachTransformRules);
		}
	}
	// OuterZone 생성해서 InnerZone에 부착
	// OuterZone = Cast<USphereComponent>(GetOwner()->
	// 	AddComponentByClass(USphereComponent::StaticClass(),false,FTransform::Identity,false));

	// NewObject로 변경함
	if (OuterZone = NewObject<USphereComponent>(this->GetOwner()
	                                            , USphereComponent::StaticClass(),TEXT("OuterZone")))
	{
		OuterZone->RegisterComponent();

		OuterZone->SetHiddenInGame(!EnableDebug);

		if (InnerZone && OuterZone)
		{
			FAttachmentTransformRules AttachTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld,
			                                               EAttachmentRule::KeepWorld, false);

			OuterZone->AttachToComponent(InnerZone, AttachTransformRules);
		}

		InnerZone->OnComponentBeginOverlap.AddDynamic(this, &UMyManager_InteractionTarget::OnInnerZoneBeginOverlap);
		InnerZone->OnComponentEndOverlap.AddDynamic(this, &UMyManager_InteractionTarget::OnInnerZoneEndOverlap);

		OuterZone->OnComponentBeginOverlap.AddDynamic(this, &UMyManager_InteractionTarget::OnOuterZoneBeginOverlap);
		OuterZone->OnComponentEndOverlap.AddDynamic(this, &UMyManager_InteractionTarget::OnOuterZoneEndOverlap);

		InnerZone->SetSphereRadius(InnerZoneRadius, true);

		OuterZone->SetSphereRadius(FMath::Max(OuterZoneExtent, 10) + InnerZoneRadius, true);
	}
}

// ComponentsToHighlight에 담긴 이름과 일치하는 Component가 있으면 HighlightedComponents에 추가
void UMyManager_InteractionTarget::ConstructHighlightedComponents()
{
	if (GetOwner())
	{
		USceneComponent* LocOwnerRoot = GetOwner()->GetRootComponent();

		if (UPrimitiveComponent* LocOwnerPrimitive = Cast<UPrimitiveComponent>(LocOwnerRoot))
		{
			if (ComponentsToHighlight.Contains(LocOwnerPrimitive->GetName()))
			{
				HighlightedComponents.AddUnique(LocOwnerPrimitive);
			}
		}
		TArray<USceneComponent*> ChildComponents;
		LocOwnerRoot->GetChildrenComponents(true, ChildComponents);
		for (USceneComponent* Child : ChildComponents)
		{
			if (UPrimitiveComponent* LocChildPrimitive = Cast<UPrimitiveComponent>(Child))
			{
				if (ComponentsToHighlight.Contains(LocChildPrimitive->GetName()))
				{
					HighlightedComponents.AddUnique(LocChildPrimitive);
				}
			}
		}
	}
}

// Find Interactor Component from Overlapped Actor and ServerUpdateInteractionTarget
// Update Widget in Client, Add InteractionTargets in server
void UMyManager_InteractionTarget::OnInnerZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                           bool bFromSweep, const FHitResult& SweepResult)
{
	if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
	{
		AController* Controller = PlayerPawn->GetController();
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red,
		                                 FString::Printf(TEXT("Player Enters InnerZone")));

		if (Controller && Controller->IsLocalPlayerController())
		{
			UMyManager_Interactor* InteractorManager = Cast<UMyManager_Interactor>(Controller->
				GetComponentByClass(UMyManager_Interactor::StaticClass()));

			if (InteractorManager)
			{
				InteractorManager->ServerUpdateInteractionTargets(true, this);
			}
		}
	}
}

void UMyManager_InteractionTarget::OnInnerZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
	{
		AController* Controller = PlayerPawn->GetController();
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red,
		                                 FString::Printf(TEXT("Player Leaves InnerZone")));
		if (Controller && Controller->IsLocalPlayerController())
		{
			UMyManager_Interactor* InteractorManager = Cast<UMyManager_Interactor>(Controller->
				GetComponentByClass(UMyManager_Interactor::StaticClass()));

			if (InteractorManager)
			{
				InteractorManager->ServerUpdateInteractionTargets(false, this);
			}
		}
	}
}

void UMyManager_InteractionTarget::OnOuterZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                           bool bFromSweep, const FHitResult& SweepResult)
{
	if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
	{
		AController* Controller = PlayerPawn->GetController();
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red,
		                                 FString::Printf(TEXT("Player Enters OuterZone")));

		if (Controller && Controller->IsLocalPlayerController())
		{
			UMyManager_Interactor* InteractorManager = Cast<UMyManager_Interactor>(Controller->
				GetComponentByClass(UMyManager_Interactor::StaticClass()));

			if (InteractorManager)
			{
				InteractorManager->ServerUpdatePointOfInterests(true, this);

				InteractorManager->ServerRequestAssignInteractor(true, this);
			}
		}
	}
}

void UMyManager_InteractionTarget::OnOuterZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
	{
		AController* Controller = PlayerPawn->GetController();
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red,
		                                 FString::Printf(TEXT("Player Leaves OuterZone")));

		if (Controller && Controller->IsLocalPlayerController())
		{
			UMyManager_Interactor* InteractorManager = Cast<UMyManager_Interactor>(Controller->
				GetComponentByClass(UMyManager_Interactor::StaticClass()));

			if (InteractorManager)
			{
				InteractorManager->ServerUpdatePointOfInterests(false, this);

				InteractorManager->ServerRequestAssignInteractor(false, this);
			}
		}
	}
}

void UMyManager_InteractionTarget::UpdateWidgetInfo(FMargin InWidgetMargin, double InScreenRadiusPercent)
{
	WidgetMargin = InWidgetMargin;

	ScreenRadiusPercent = InScreenRadiusPercent;
}

void UMyManager_InteractionTarget::SelectMarkerComponent(FString ComponentName)
{
	if (GetOwner())
	{
		USceneComponent* LocOwnerRoot = GetOwner()->GetRootComponent();

		TArray<USceneComponent*> LocComponents;
		LocOwnerRoot->GetChildrenComponents(true, LocComponents);

		LocComponents.Add(LocOwnerRoot);

		for (USceneComponent* ChildComponent : LocComponents)
		{
			if (MarkerComponentName == ChildComponent->GetName())
			{
				MarkerTargetComponent = ChildComponent;
				break;
			}
		}

		if (!MarkerTargetComponent)
		{
			MarkerTargetComponent = LocOwnerRoot;
		}
	}
}

void UMyManager_InteractionTarget::SetHighlight(bool IsHighlighted)
{
	for (UPrimitiveComponent* Component : HighlightedComponents)
	{
		Component->SetRenderCustomDepth(IsHighlighted);
	}
}

bool UMyManager_InteractionTarget::IsReactivationEnabled()
{
	switch (FinishMethod)
	{
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompleted:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceled:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedOrCanceled:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceledDestroyOnCompleted:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceledDeactivateOnCompleted:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedDestroyOnCanceled:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedDeactivateOnCanceled:
	case Enum_InteractionFinishMethod::DeactivateOnCanceledReactivateAfterDurationOnCompleted:
	case Enum_InteractionFinishMethod::DeactivateOnCompletedReactivateAfterDurationOnCanceled:
		return true;
	default:
		return false;
	}
}

bool UMyManager_InteractionTarget::CancelOnRelease()
{
	switch (FinishMethod)
	{
	case Enum_InteractionFinishMethod::DestroyOnCanceled:
	case Enum_InteractionFinishMethod::DestroyOnCompletedOrCanceled:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceled:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedOrCanceled:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceledDestroyOnCompleted:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCanceledDeactivateOnCompleted:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedDestroyOnCanceled:
	case Enum_InteractionFinishMethod::ReactivateAfterDurationOnCompletedDeactivateOnCanceled:
	case Enum_InteractionFinishMethod::DeactivateOnCanceled:
	case Enum_InteractionFinishMethod::DeactivateOnCompletedOrCanceled:
	case Enum_InteractionFinishMethod::DeactivateOnCanceledDestroyOnCompleted:
	case Enum_InteractionFinishMethod::DeactivateOnCanceledReactivateAfterDurationOnCompleted:
	case Enum_InteractionFinishMethod::DeactivateOnCompletedDestroyOnCanceled:
	case Enum_InteractionFinishMethod::DeactivateOnCompletedReactivateAfterDurationOnCanceled:
		return true;
		break;
	default:
		return false;
		break;
	}
}

// 여기서 위젯을 지워야할까?
// Deactivated Targets에 들어가있다.
// ClearWidgetInteractionTarget?
// DeactivatedTargets를 관리하는 방식이 필요할것같다.
void UMyManager_InteractionTarget::OnDeactivated()
{
	InnerZone->SetGenerateOverlapEvents(false);

	OuterZone->SetGenerateOverlapEvents(false);

	InnerZone->SetHiddenInGame(true);

	OuterZone->SetHiddenInGame(true);
}

void UMyManager_InteractionTarget::OnAddedToPendingTarget()
{
	UMyManager_Interactor* LocCurrentInteractor = nullptr;
	LastInteractedTime = UKismetSystemLibrary::GetGameTimeInSeconds(this);

	for (AController* Interactor : AssignedInteractors)
	{
		LocCurrentInteractor = GetInteractorManager(Interactor);

		LocCurrentInteractor->AddToPendingTargets(this);
	}

	InteractionEnabled = false;
}

void UMyManager_InteractionTarget::OnInteractionBeginEvent(APawn* InteractorPawn)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (NetworkHandleMethod == Enum_InteractionNetworkHandleMethod::DisableWhileInteracting)
		{
			for (AController* Interactor : AssignedInteractors)
			{
				if (Interactor->GetPawn() != InteractorPawn)
				{
					if (UMyManager_Interactor* TempInteractor = Cast<UMyManager_Interactor>(
						Interactor->GetComponentByClass(UMyManager_Interactor::StaticClass())))
					{
						TempInteractor->AddToDeactivatedTargets(this);
					}
				}
			}
		}
	}
}

void UMyManager_InteractionTarget::OnInteractionEndEvent(Enum_InteractionResult Result, APawn* InteractorPawn)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (NetworkHandleMethod == Enum_InteractionNetworkHandleMethod::DisableWhileInteracting)
		{
			for (AController* Interactor : AssignedInteractors)
			{
				if (Interactor->GetPawn() != InteractorPawn)
				{
					if (UMyManager_Interactor* LocCurrentInteractor = Cast<UMyManager_Interactor>(
						Interactor->GetComponentByClass(UMyManager_Interactor::StaticClass())))
					{
						if (LocCurrentInteractor->DeactivatedTargets.Contains(this))
						{
							LocCurrentInteractor->RemoveFromDeactivatedTargets(this);
							LocCurrentInteractor->ApplyFinishMethod(this, Result);
						}
					}
				}
			}
		}
	}
}

void UMyManager_InteractionTarget::AssignInteractor(bool Add, AController* Interactor)
{
	if (Add)
	{
		AssignedInteractors.AddUnique(Interactor);
	}
	else
	{
		AssignedInteractors.Remove(Interactor);
	}
}

void UMyManager_InteractionTarget::EnableInteraction(bool Enable)
{
	InteractionEnabled = Enable;

	if (Enable)
	{
		for (AController* Interactor : AssignedInteractors)
		{
			UMyManager_Interactor* InteractorManager = GetInteractorManager(Interactor);

			InteractorManager->RemoveFromDeactivatedTargets(this);

			InteractorManager->OnInteractionTargetReactivated(this);
		}
	}
	else
	{
		for (AController* Interactor : AssignedInteractors)
		{
			UMyManager_Interactor* InteractorManager = GetInteractorManager(Interactor);

			InteractorManager->AddToDeactivatedTargets(this);
		}
	}
}
