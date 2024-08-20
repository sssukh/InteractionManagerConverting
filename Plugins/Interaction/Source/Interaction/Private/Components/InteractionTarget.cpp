
#include "Components/InteractionTarget.h"

#include "InteractionLog.h"
#include "Components/InteractorManager.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Objects/DestroyOnFinish.h"
#include "Objects/InteractionFinish.h"
#include "Objects/ReactivateOnFinish.h"


UInteractionTarget::UInteractionTarget()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// ...

	HighlightColor = FLinearColor(1, 1, 1, 1);
	InteractionText.Add(EInteractionState::Waiting, FText::FromString(TEXT("Waiting")));
	InteractionText.Add(EInteractionState::Interacting, FText::FromString(TEXT("Interacting...")));
	InteractionText.Add(EInteractionState::Done, FText::FromString(TEXT("Done")));

	POI_IconColor = FLinearColor(0.5, 0.5, 0.5, 1.0f);
	Target_IconColor = FLinearColor(0.8, 0.8, 0.8, 1.0f);
	Interaction_Icon_Background_Color = FLinearColor(0.2, 0.2, 0.2, 1.0);
	Interaction_Icon_Foreground_Color = FLinearColor(1, 1, 1, 1);


	MarkerComponentName = FString("MarkerTarget");

	static ConstructorHelpers::FObjectFinder<UTexture2D> T_CircleObject(TEXT("/Script/Engine.Texture2D'/Interaction/UserInterface/Textures/T_Circle.T_Circle'"));
	if (T_CircleObject.Succeeded())
	{
		POI_Icon.Texture2D = T_CircleObject.Object;
		Interaction_Icon_Background.Texture2D = T_CircleObject.Object;;
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> MI_CircularProgressBar(
		TEXT("/Script/Engine.MaterialInstanceConstant'/Interaction/UserInterface/Materials/MI_CircularProgressBar.MI_CircularProgressBar'"));
	if (MI_CircularProgressBar.Succeeded())
	{
		Interaction_Icon_Foreground.Material = MI_CircularProgressBar.Object;
	}
}

void UInteractionTarget::BeginPlay()
{
	Super::BeginPlay();

	ConstructOwnerEssentials();
	ConstructOverlapZones();
	ConstructHighlightedComponents();
	SelectMarkerComponent();
}

void UInteractionTarget::ConstructOwnerEssentials()
{
	OwnerReference = GetOwner();
}

void UInteractionTarget::ConstructOverlapZones()
{
	// 상호작용 대상의 모든 SceneComponent를 가져옵니다.
	TArray<USceneComponent*> SceneComponentArray;
	GetOwner()->GetComponents<USceneComponent>(SceneComponentArray);

	// 모든 SceneComponent 중에서 InteractionZoneComponentToAttach 이름과 일치하는 컴포넌트를 찾습니다.
	USceneComponent* LocComponentToAttach = nullptr;
	for (USceneComponent* SceneComponent : SceneComponentArray)
	{
		if (GetNameSafe(SceneComponent) == InteractionZoneComponentToAttach)
		{
			LocComponentToAttach = SceneComponent;
			break;
		}
	}

	// 만약 특정 컴포넌트를 찾지 못했을 경우, 루트 컴포넌트를 기본 값으로 설정합니다.
	if (!LocComponentToAttach)
	{
		LOG_WARNING_AND_SCREEN(5.0f, TEXT("InteractionZoneComponentToAttach가 설정되지 않아서 LocComponentToAttach가 존재하지 않습니다. 기본 설정 루트컴포넌트에 어태치 됩니다."))
		LocComponentToAttach = GetOwner()->GetRootComponent();
	}

	FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget,
	                                                                               EAttachmentRule::KeepWorld,
	                                                                               EAttachmentRule::KeepWorld,
	                                                                               false);


	// USphereComponent를 동적으로 생성합니다.
	if (USphereComponent* NewSphereComponent = NewObject<USphereComponent>(this, USphereComponent::StaticClass(), TEXT("InnerZone")))
	{
		InnerZone = NewSphereComponent;
		InnerZone->AttachToComponent(LocComponentToAttach, AttachmentTransformRules);
		InnerZone->RegisterComponent();

		InnerZone->SetHiddenInGame(!bEnableDebug);
		InnerZone->SetSphereRadius(InnerZoneRadius);

		InnerZone->OnComponentBeginOverlap.AddDynamic(this, &UInteractionTarget::OnInnerZoneBeginOverlap);
		InnerZone->OnComponentEndOverlap.AddDynamic(this, &UInteractionTarget::OnInnerZoneEndOverlap);

		GetOwner()->AddInstanceComponent(InnerZone);
	}


	if (USphereComponent* NewSphereComponent = NewObject<USphereComponent>(this, USphereComponent::StaticClass(), TEXT("OuterZone")))
	{
		OuterZone = NewSphereComponent;
		OuterZone->AttachToComponent(InnerZone, AttachmentTransformRules);
		OuterZone->RegisterComponent();

		OuterZone->SetHiddenInGame(!bEnableDebug);
		OuterZone->SetSphereRadius(InnerZoneRadius + FMath::Max(OuterZoneExtent, 100));

		OuterZone->OnComponentBeginOverlap.AddDynamic(this, &UInteractionTarget::OnOuterZoneBeginOverlap);
		OuterZone->OnComponentEndOverlap.AddDynamic(this, &UInteractionTarget::OnOuterZoneEndOverlap);

		GetOwner()->AddInstanceComponent(OuterZone);
	}
}

void UInteractionTarget::ConstructHighlightedComponents()
{
	USceneComponent* OwnerRootComponent = GetOwner()->GetRootComponent();
	if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(OwnerRootComponent))
	{
		if (ComponentsToHighlight.Contains(GetNameSafe(PrimitiveComponent)))
			HighlightedComponents.Add(PrimitiveComponent);
	}

	TArray<USceneComponent*> ChildrenComponents;
	OwnerRootComponent->GetChildrenComponents(true, ChildrenComponents);
	for (USceneComponent* Child : ChildrenComponents)
	{
		UPrimitiveComponent* ChildPrimitiveComponent = Cast<UPrimitiveComponent>(Child);
		if (!ChildPrimitiveComponent)
			continue;

		if (ComponentsToHighlight.Contains(GetNameSafe(ChildPrimitiveComponent)))
			HighlightedComponents.Add(ChildPrimitiveComponent);
	}
}

void UInteractionTarget::SelectMarkerComponent()
{
	USceneComponent* OwnerRootComponent = GetOwner()->GetRootComponent();

	TArray<USceneComponent*> ChildrenComponents;
	OwnerRootComponent->GetChildrenComponents(true, ChildrenComponents);
	ChildrenComponents.Add(OwnerRootComponent);

	// 모든 자식 컴포넌트와 루트 컴포넌트를 반복하여 확인합니다.
	for (USceneComponent* Child : ChildrenComponents)
	{
		// 자식 컴포넌트의 이름이 MarkerComponentName과 일치하는지 확인합니다.
		if (MarkerComponentName == GetNameSafe(Child))
		{
			MarkerTargetComponent = Child;
			break;
		}
	}

	// MarkerTargetComponent가 유효하지 않은 경우(즉, 지정된 이름을 가진 컴포넌트를 찾지 못한 경우), 루트 컴포넌트를 MarkerTargetComponent로 설정합니다.
	if (!IsValid(MarkerTargetComponent))
		MarkerTargetComponent = OwnerRootComponent;
}

void UInteractionTarget::AssignInteractor(bool bIsAdd, APlayerController* AssignedController)
{
	if (bIsAdd)
	{
		AssignedInteractors.AddUnique(AssignedController);
	}
	else
	{
		AssignedInteractors.Remove(AssignedController);
	}
}

void UInteractionTarget::OnDeactivated()
{
	InnerZone->SetGenerateOverlapEvents(false);
	InnerZone->SetHiddenInGame(true);

	OuterZone->SetGenerateOverlapEvents(false);
	OuterZone->SetHiddenInGame(true);
}

void UInteractionTarget::OnInnerZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                 const FHitResult& SweepResult)
{
	UInteractionManager* ManagerInteractor;
	if (!TryGetInteractorComponents(OtherActor, ManagerInteractor))
		return;

	ManagerInteractor->ServerUpdateInteractionTargets(true, this);
}

void UInteractionTarget::OnInnerZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UInteractionManager* ManagerInteractor;
	if (!TryGetInteractorComponents(OtherActor, ManagerInteractor))
		return;

	ManagerInteractor->ServerUpdateInteractionTargets(false, this);
}

void UInteractionTarget::OnOuterZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                 const FHitResult& SweepResult)
{
	UInteractionManager* ManagerInteractor;
	if (!TryGetInteractorComponents(OtherActor, ManagerInteractor))
		return;

	ManagerInteractor->ServerUpdatePointOfInterests(true, this);
	ManagerInteractor->ServerRequestAssignInteractor(true, this);
}

void UInteractionTarget::OnOuterZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UInteractionManager* ManagerInteractor;

	if (!TryGetInteractorComponents(OtherActor, ManagerInteractor))
		return;

	ManagerInteractor->ServerUpdatePointOfInterests(false, this);
	ManagerInteractor->ServerRequestAssignInteractor(false, this);
}

bool UInteractionTarget::TryGetInteractorComponents(AActor* OtherActor, UInteractionManager*& OutManagerInteractor)
{
	APawn* OverlappedPawn = Cast<APawn>(OtherActor);
	if (!OverlappedPawn)
		return false;

	AController* PawnController = OverlappedPawn->GetController();
	if (!PawnController)
		return false;

	if (!PawnController->IsLocalPlayerController())
		return false;

	if (UInteractionManager* ManagerInteractor = PawnController->GetComponentByClass<UInteractionManager>())
	{
		OutManagerInteractor = ManagerInteractor;
		return true;
	}

	return false;
}

void UInteractionTarget::OnInteractionBeginEvent(APawn* InInteractorPawn)
{
	if (!GetOwner()->HasAuthority())
		return;

	// 네트워크 처리 방법이 상호작용 중 비활성화(DisableWhileInteracting)로 설정된 경우
	if (NetworkHandleMethod == EInteractionNetworkHandleMethod::DisableWhileInteracting)
	{
		// 할당된 모든 상호작용자(AssignedInteractors)를 순회하며 처리
		for (AController* AssignedController : AssignedInteractors)
		{
			// 상호작용을 시작한 Pawn과 같은 Pawn을 가진 Interactor는 처리하지 않음
			if (AssignedController->GetPawn() == InInteractorPawn)
				continue;

			UInteractionManager* ManagerInteractor = AssignedController->GetPawn()->GetComponentByClass<UInteractionManager>();
			if (!ManagerInteractor)
				continue;

			// 상호작용 대상이 비활성화된 타겟 리스트에 추가
			ManagerInteractor->AddToDeactivatedTargets(this);
		}
	}
}

void UInteractionTarget::OnInteractionEndEvent(EInteractionResult InInteractionResult, APawn* InInteractorPawn)
{
	if (!GetOwner()->HasAuthority())
		return;

	// 네트워크 처리 방법이 상호작용 중 비활성화(DisableWhileInteracting)로 설정된 경우
	if (NetworkHandleMethod == EInteractionNetworkHandleMethod::DisableWhileInteracting)
	{
		// 할당된 모든 상호작용자(AssignedInteractors)를 순회하며 처리
		for (AController* AssignedController : AssignedInteractors)
		{
			if (AssignedController->GetPawn() != InInteractorPawn)
				continue;

			// 현재 상호작용자를 위한 InteractorManager 컴포넌트를 가져옴
			UInteractionManager* ManagerInteractor = AssignedController->GetComponentByClass<UInteractionManager>();
			if (!ManagerInteractor->DeactivatedTargets.Contains(this)) continue;

			// 상호작용 종료 방법에 따라 필요한 추가 처리를 수행
			ManagerInteractor->ApplyFinishMethod(this, InInteractionResult);

			// 비활성화된 타겟 리스트에서 현재 타겟을 제거
			ManagerInteractor->RemoveFromDeactivatedTargets(this);
		}
	}
}

void UInteractionTarget::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UInteractionTarget::IsInteractionEnabled()
{
	return bInteractionEnabled;
}

bool UInteractionTarget::IsReactivationEnabled()
{
	if (InteractionFinishInstance && InteractionFinishInstance->IsA<UReactivateOnFinish>())
	{
		return true;
	}

	return false;
}

bool UInteractionTarget::CancelOnRelease()
{
	if (InteractionFinishInstance && InteractionFinishInstance->bIsCancel)
	{
		return true;
	}

	return false;
}

void UInteractionTarget::SetHighlight(bool bIsHighlighted)
{
	for (UPrimitiveComponent* HighlightedComponent : HighlightedComponents)
	{
		HighlightedComponent->SetRenderCustomDepth(bIsHighlighted);
	}
}

void UInteractionTarget::UpdateWidgetInfo(FMargin InWidgetMargin, float InScreenRadiusPercent)
{
	WidgetMargin = InWidgetMargin;
	ScreenRadiusPercent = InScreenRadiusPercent;
}
