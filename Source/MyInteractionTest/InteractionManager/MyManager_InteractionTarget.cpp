// Fill out your copyright notice in the Description page of Project Settings.


#include "MyManager_InteractionTarget.h"

#include "MyManager_Interactor.h"
#include "Components/SphereComponent.h"


// Sets default values for this component's properties
UMyManager_InteractionTarget::UMyManager_InteractionTarget()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMyManager_InteractionTarget::BeginPlay()
{
	Super::BeginPlay();

	ConstructOwnerEssentials();

	ConstructOverlapZones();

	ConstructHighlightedComponents();

	SelectMarkerComponent(MarkerComponentName);
	// ...
	
}

void UMyManager_InteractionTarget::ConstructOwnerEssentials()
{
	if(GetOwner())
	{
		OwnerReference = GetOwner();
	}
}

void UMyManager_InteractionTarget::ConstructOverlapZones()
{
	
	USceneComponent* LocComponentToAttach = nullptr;
	
	TArray<USceneComponent*> SceneArray;
	GetOwner()->GetComponents(USceneComponent::StaticClass(),SceneArray);

	// InnerZone과 OuterZone을 부착할 Component를 이름으로 찾기
	for (USceneComponent* Scene : SceneArray)
	{
		if(Scene->GetName()==InteractionZone_ComponentToAttach)
		{
			LocComponentToAttach = Scene;
			break;
		}
	}

	// InnerZone 생성해서 위에서 찾은 Component에 부착
	InnerZone = Cast<USphereComponent>(GetOwner()->
		AddComponentByClass(USphereComponent::StaticClass(),false,FTransform::Identity,false));
	

	InnerZone->SetHiddenInGame(!EnableDebug);

	if(LocComponentToAttach&&InnerZone)
	{
		FAttachmentTransformRules AttachTransformRules(EAttachmentRule::SnapToTarget,EAttachmentRule::KeepWorld,EAttachmentRule::KeepWorld,false);
		
		InnerZone->AttachToComponent(LocComponentToAttach,AttachTransformRules);
	}

	// OuterZone 생성해서 InnerZone에 부착
	OuterZone = Cast<USphereComponent>(GetOwner()->
		AddComponentByClass(USphereComponent::StaticClass(),false,FTransform::Identity,false));
	

	OuterZone->SetHiddenInGame(!EnableDebug);

	if(InnerZone&&OuterZone)
	{
		FAttachmentTransformRules AttachTransformRules(EAttachmentRule::SnapToTarget,EAttachmentRule::KeepWorld,EAttachmentRule::KeepWorld,false);
		
		OuterZone->AttachToComponent(InnerZone,AttachTransformRules);
	}

	InnerZone->OnComponentBeginOverlap.AddDynamic(this,&UMyManager_InteractionTarget::OnInnerZoneBeginOverlap);
	InnerZone->OnComponentEndOverlap.AddDynamic(this,&UMyManager_InteractionTarget::OnInnerZoneEndOverlap);

	OuterZone->OnComponentBeginOverlap.AddDynamic(this,&UMyManager_InteractionTarget::OnOuterZoneBeginOverlap);
	OuterZone->OnComponentEndOverlap.AddDynamic(this,&UMyManager_InteractionTarget::OnOuterZoneEndOverlap);	
}

void UMyManager_InteractionTarget::ConstructHighlightedComponents()
{
}

void UMyManager_InteractionTarget::OnInnerZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AController* Controller = nullptr;
	if(OtherActor)
	{
		Controller = Cast<APawn>(OtherActor)->GetController();
	}
	
	if(Controller&&Controller->IsLocalPlayerController())
	{
		UMyManager_Interactor* InteractorManager = Cast<UMyManager_Interactor>(Controller->
			GetComponentByClass(UMyManager_Interactor::StaticClass()));

		if(InteractorManager)
		{
			InteractorManager->ServerUpdateInteractionTargets(true,this);
		}
	}
}

void UMyManager_InteractionTarget::OnInnerZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AController* Controller = nullptr;
	if(OtherActor)
	{
		Controller = Cast<APawn>(OtherActor)->GetController();
	}
	if(Controller&&Controller->IsLocalPlayerController())
	{
		UMyManager_Interactor* InteractorManager = Cast<UMyManager_Interactor>(Controller->
			GetComponentByClass(UMyManager_Interactor::StaticClass()));

		if(InteractorManager)
		{
			InteractorManager->ServerUpdateInteractionTargets(false,this);
		}
	}
}

void UMyManager_InteractionTarget::OnOuterZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AController* Controller = nullptr;
	if(OtherActor)
	{
		Controller = Cast<APawn>(OtherActor)->GetController();
	}
	
	if(Controller&&Controller->IsLocalPlayerController())
	{
		UMyManager_Interactor* InteractorManager = Cast<UMyManager_Interactor>(Controller->
			GetComponentByClass(UMyManager_Interactor::StaticClass()));

		if(InteractorManager)
		{
			// InteractorManager->ServerUpdatePointOfInterests(true,this);

			// InteractorManager->ServerRequestAssignInteractor(this,true);
		}
	}
}

void UMyManager_InteractionTarget::OnOuterZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AController* Controller = nullptr;
	if(OtherActor)
	{
		Controller = Cast<APawn>(OtherActor)->GetController();
	}
}

void UMyManager_InteractionTarget::SelectMarkerComponent(FString ComponentName)
{
}

void UMyManager_InteractionTarget::SetHighlight(bool IsHighlighted)
{
	for (UPrimitiveComponent* Component : HighlightedComponents)
	{
		Component->SetRenderCustomDepth(IsHighlighted);
	}
}




