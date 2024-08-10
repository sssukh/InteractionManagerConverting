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

	// virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// Network Events
public:
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

	// Network
public:
	
	
	
	
protected:
	UFUNCTION()
	void OnInteractionTargetUpdatedServerSide(bool Add,UMyManager_InteractionTarget* InteractionTarget);
	
	UFUNCTION()
	void OnInteractionTargetUpdatedClientSide(bool Add,UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION()
	void OnPointOfInterestUpdatedServerSide(bool Add,UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION()
	void OnPointOfInterestUpdatedClientSide(bool Add,UMyManager_InteractionTarget* InteractionTarget);
	// Main
protected:
	UFUNCTION()
	void ConstructPlayerEssentials();

	UFUNCTION()
	void ConstructPooledMarkerWidgets();

	UFUNCTION()
	void ConstructPostProcessComponent();

	UFUNCTION()
	void Update_InteractionKeys();
	
	UFUNCTION()
	void Debug_Function();

	//Interactable
protected:
	UFUNCTION()
	UUW_InteractionTarget* FindEmptyWidget();
	
	UFUNCTION()
	bool IsInteractable(UMyManager_InteractionTarget* ItemToFind);
	
	UFUNCTION()
	UUW_InteractionTarget* FindWidgetByInteractionTarget(UMyManager_InteractionTarget* InteractionTarget);
	
	// UFUNCTION()
	// bool GetInteractionKeys(TArray<FKey>& ReturnKeyRef) const;
	//
	// UFUNCTION()
	// UMyManager_InteractionTarget* Find_Best_Interactable();
	//
	// UFUNCTION()
	// void Update_Best_Interactable(UMyManager_InteractionTarget* NewTarget);
	
protected:
	UPROPERTY()
	TSubclassOf<UUW_InteractionTarget> InteractionWidgetClass;

	// Interaction
public:
	UFUNCTION()
	void ApplyFinishMethod(UMyManager_InteractionTarget* InteractionTarget,Enum_InteractionResult Result);
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
	
	// UFUNCTION()
	// void OnNewTargetSelectedClientSide(UMyManager_InteractionTarget* NewTarget, bool IsSelected);
	//
	// UFUNCTION()
	// void SetTargetHighlighted(UMyManager_InteractionTarget* InteractionTarget, bool IsHighlighted);

	
	// Variables
	// Data
public:
	
	// InteractionTarget 으로 교체하기?
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray<UMyManager_InteractionTarget*> InteractionTargets;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray<UMyManager_InteractionTarget*> PointOfInterests;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray<UMyManager_InteractionTarget*> PendingTargets;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray<UMyManager_InteractionTarget*> DeactivatedTargets;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray<UUW_InteractionTarget*> WidgetPool;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TObjectPtr<APlayerController> OwnerController;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TObjectPtr<UPostProcessComponent> PostProcessComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TObjectPtr<UUW_InteractionTarget> CurrentInteractionMarker;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TObjectPtr<UMyManager_InteractionTarget> BestInteractionTarget;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor")
	TArray<FKey> InteractionKeys;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor")
	double CurrentHoldTime;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor")
	double RepeatCooldown;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor")
	int32 Repeated;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor")
	bool IsInteracting;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor")
	bool KeyJustPressed;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor")
	bool IsGamePad;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Manager Interactor")
	FKey LastPressedKey;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	FTimerHandle BeginUpdateKeysTimerHandle;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	FTimerHandle PendingTargetTimerHandle;
	
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TObjectPtr<UMaterialInstanceDynamic> Outline_DynamicMaterial;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TObjectPtr<UMaterialInterface> m_OutlineMaterial;

	// Main
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactor Manager|Main")
	bool Debug = true;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactor Manager|Main")
	int32 DefaultWidgetPoolSize = 3;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactor Manager|Main")
	double PendingTargetCheckInteraval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Interactor Manager|Main",meta = (AllowPrivateAccess=true))
	TObjectPtr<UInputAction> InteractionInputAction;

	// Marker Setting
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactor Manager|Marker Setting")
	FMargin WidgetScreenMargin;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactor Manager|Marker Setting")
	double ScreenRadiusPercent;
};
