// Fill out your copyright notice in the Description page of Project Settings.


#include "MyManager_Interactor.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PostProcessComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyManager_InteractionTarget.h"
#include "Chaos/Utilities.h"
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
}


// Called when the game starts
void UMyManager_Interactor::BeginPlay()
{
	Super::BeginPlay();

	ConstructPlayerEssentials();

	ConstructPooledMarkerWidgets();
	// ...

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
	if(!InteractionWidgetClass)
	{
		//ToDo 로그 추가해주세요.
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
	PostProcessComponent = Cast<UPostProcessComponent>(OwnerController->GetPawn()->
		AddComponentByClass(UPostProcessComponent::StaticClass(),true,FTransform::Identity,false));

	Outline_DynamicMaterial  = UMaterialInstanceDynamic::Create(m_OutlineMaterial, this );

	FWeightedBlendable TempWeightedBlendable;
	TempWeightedBlendable.Object = Outline_DynamicMaterial;
	TempWeightedBlendable.Weight = 1.0f;
	
	PostProcessComponent->Settings.WeightedBlendables.Array.Add(TempWeightedBlendable);
}

void UMyManager_Interactor::Update_InteractionKeys()
{
	if(OwnerController->IsLocalPlayerController())
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwnerController->GetLocalPlayer()))
		{
			InteractionKeys = Subsystem->QueryKeysMappedToAction(InteractionInputAction);
			GetWorld()->GetTimerManager().ClearTimer(BeginUpdateKeysTimerHandle);
		}
	}
}




// void UMyManager_Interactor::Debug_Function()
// {
// 	if(Debug)
// 	{
// 		if(GetOwner()->HasAuthority())
// 		{
// 			GEngine->AddOnScreenDebugMessage(0,0.0f,FColor::Blue,TEXT("Interaction Targets : %d",InteractionTargets.Num()));
// 			GEngine->AddOnScreenDebugMessage(0,0.0f,FColor::Blue,TEXT("Point Of Interests : %d"),PointOfInterests.Num());
// 			GEngine->AddOnScreenDebugMessage(0,0.0f,FColor::Blue,(TEXT("Pending Targets : %d"),PendingTargets.Num()));
// 			GEngine->AddOnScreenDebugMessage(0,0.0f,FColor::Blue,(TEXT("Marker : %s"),*CurrentInteractionMarker->GetName()));
// 			
// 		}
// 	}
// }



UMyManager_InteractionTarget* UMyManager_Interactor::Find_Best_Interactable()
{
	double LocDotProduct =0.0;

	UMyManager_InteractionTarget* LocTargetInteractable = nullptr;
	for (UMyManager_InteractionTarget* LocCachedInteractable : InteractionTargets)
	{
		// IsInteractable? 여기있음
		// PendingTargets와 DeactivatedTargets에 LocCachedInteractable이 없고 멤버변수 InteractionEnabled가 true이면
		if(!(PendingTargets.Contains(LocCachedInteractable)||DeactivatedTargets.Contains(LocCachedInteractable))
			&&LocCachedInteractable->IsInteractionEnabled())
		{
			FVector LocActorOrigin = LocCachedInteractable->MarkerTargetComponent->GetComponentLocation()
									+ LocCachedInteractable->MarkerTargetComponent->GetComponentRotation().RotateVector(LocCachedInteractable->MarkerOffset);

			FVector SubResult = LocActorOrigin-OwnerController->PlayerCameraManager->GetCameraLocation();

			FVector PlayerCameraManagerForwardVector = OwnerController->PlayerCameraManager->GetCameraRotation().Vector();

			double DotResult = FVector::DotProduct(SubResult.GetSafeNormal(0.0001),PlayerCameraManagerForwardVector);

			if(DotResult>0.5&&DotResult>LocDotProduct)
			{
				LocDotProduct = DotResult;
				LocTargetInteractable = LocCachedInteractable;
			}
		}
	}

	return LocTargetInteractable;
}


