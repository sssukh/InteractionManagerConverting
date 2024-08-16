// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enums/InteractionManagerEnums.h"
#include "MyManager_Interactor.generated.h"

class UUW_InteractionTarget;
class UPostProcessComponent;
class UUserWidget;
class UInputAction;
class UMyManager_InteractionTarget;

UCLASS(BlueprintType,Blueprintable,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYINTERACTIONTEST_API UMyManager_Interactor : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMyManager_Interactor();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// Network Events
public:
	UFUNCTION(Unreliable, Client)
	void ClientCheckPressedKey();
	
	UFUNCTION(Unreliable, Client)
	void ClientSetNewTarget(UMyManager_InteractionTarget* NewTarget,bool IsSelected);

	UFUNCTION(Unreliable,Server, WithValidation)
	void ServerOnInteractionFinished(UMyManager_InteractionTarget* InteractionTarget,Enum_InteractionResult Result);
	
	UFUNCTION(Unreliable,Server, WithValidation)
	void ServerUpdateInteractionTargets(bool Add,UMyManager_InteractionTarget* InteractionTarget);
	
	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerUpdatePointOfInterests(bool Add, UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION(Unreliable, Client)
	void ClientResetData();
	
	UFUNCTION(Unreliable,Server, WithValidation)
	void ServerRequestAssignInteractor(bool Add,UMyManager_InteractionTarget* InteractionTarget);
	
	UFUNCTION(Unreliable, Client)
	void ClientUpdatePointOfInterests(bool Add, UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION(Unreliable,Client)
	void ClientUpdateInteractionTargets(bool Add,UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION(Unreliable,Server, WithValidation)
	void ServerOnInteractionBegin(UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION(Unreliable,Server, WithValidation)
	void ServerOnInteractionUpdated(UMyManager_InteractionTarget* InteractionTarget,double Alpha,int32 InRepeated,APawn* InteractorPawn);
	
	UFUNCTION()
	void OnInteractionUpdated(UMyManager_InteractionTarget* InteractionTarget,double Alpha, int32 InRepeated);
	
	UFUNCTION(Reliable,Client)
	void ClientOnInteractionTargetDestroyed(UMyManager_InteractionTarget* InteractionTarget);
	// Main
protected:
	UFUNCTION()
	void ConstructPlayerEssentials();

	UFUNCTION()
	void ConstructPooledMarkerWidgets();

	UFUNCTION()
	void ConstructPostProcessComponent();
	
	UFUNCTION()
	void Debug_Function();

protected:
	void HandleTryRepeat();

	//Interactable
protected:
	UFUNCTION()
	UUW_InteractionTarget* FindEmptyWidget();
	
	UFUNCTION()
	UUW_InteractionTarget* FindWidgetByInteractionTarget(UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION()
	bool IsInteractable(UMyManager_InteractionTarget* ItemToFind);
	
	UFUNCTION()
	bool GetInteractionKeys(TArray<FKey>& ReturnKeyRef) const;
	
	UFUNCTION()
	UMyManager_InteractionTarget* FindBestInteractable();
	
	UFUNCTION()
	void UpdateBestInteractable(UMyManager_InteractionTarget* NewTarget);

	UFUNCTION()
	void UpdateInteractionKeys();
	
	// Interaction
public:
	UFUNCTION()
	void TryTakeInteraction();
	
	UFUNCTION()
	void ApplyFinishMethod(UMyManager_InteractionTarget* InteractionTarget,Enum_InteractionResult Result);

	UFUNCTION()
	void ReceiveAnyKey(FKey Key);
	
	// Targets
public:
	UFUNCTION()
	void OnInteractionTargetDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void AddToPendingTargets(UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION()
	void OnInteractionTargetReactivated(UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION()
	void AddToDeactivatedTargets(UMyManager_InteractionTarget* InteractionTarget);
	
	UFUNCTION()
	void CheckForPendingTargets();

	UFUNCTION()
	void RemoveFromDeactivatedTargets(UMyManager_InteractionTarget* InteractionTarget);
	
	UFUNCTION()
	void OnNewTargetSelectedClientSide(UMyManager_InteractionTarget* NewTarget, bool IsSelected);
	
	UFUNCTION()
	void SetTargetHighlighted(UMyManager_InteractionTarget* InteractionTarget, bool IsHighlighted);

	// Network
public:
	UFUNCTION()
	void OnInteractionTargetUpdatedServerSide(bool Add,UMyManager_InteractionTarget* InteractionTarget);
	
	UFUNCTION()
	void OnInteractionTargetUpdatedClientSide(bool Add,UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION()
	void OnPointOfInterestUpdatedServerSide(bool Add,UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION()
	void OnPointOfInterestUpdatedClientSide(bool Add,UMyManager_InteractionTarget* InteractionTarget);
	
	// Variables
	// Data
public:
	
	// InteractionTarget 으로 교체하기?
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TArray<UMyManager_InteractionTarget*> InteractionTargets;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TArray<UMyManager_InteractionTarget*> PointOfInterests;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TArray<UMyManager_InteractionTarget*> PendingTargets;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TArray<UMyManager_InteractionTarget*> DeactivatedTargets;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TArray<UUW_InteractionTarget*> WidgetPool;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TObjectPtr<APlayerController> OwnerController;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TObjectPtr<UPostProcessComponent> PostProcessComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TObjectPtr<UUW_InteractionTarget> CurrentInteractionMarker;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TObjectPtr<UMyManager_InteractionTarget> BestInteractionTarget;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TArray<FKey> InteractionKeys;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	double CurrentHoldTime;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	double RepeatCooldown;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	int32 Repeated;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	bool IsInteracting;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	bool KeyJustPressed;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	bool IsGamePad;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	FKey LastPressedKey;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	FTimerHandle BeginUpdateKeysTimerHandle;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	FTimerHandle PendingTargetTimerHandle;
	
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor|Data")
	TObjectPtr<UMaterialInstanceDynamic> Outline_DynamicMaterial;



	// Main
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactor Manager|Main")
	bool Debug = true;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactor Manager|Main")
	int32 DefaultWidgetPoolSize;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactor Manager|Main")
	double PendingTargetCheckInteraval=0.35;


	
	// Marker Setting
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactor Manager|Marker Setting")
	FMargin WidgetScreenMargin;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactor Manager|Marker Setting")
	double ScreenRadiusPercent=0.5f;

public:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Manager Interactor|Config|Essential")
	TSubclassOf<UUW_InteractionTarget> InteractionTargetWidgetBP;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Manager Interactor|Config|Essential")
	TObjectPtr<UMaterialInterface> m_OutlineMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Manager Interactor|Config|Essential",meta = (AllowPrivateAccess=true))
	TObjectPtr<UInputAction> InteractionInputAction;;
};
