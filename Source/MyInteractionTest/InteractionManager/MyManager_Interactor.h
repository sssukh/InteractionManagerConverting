// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	UFUNCTION(Reliable,Server, WithValidation)
	void ServerUpdateInteractionTargets(bool Add,UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION(Reliable,Client)
	void ClientUpdateInteractionTargets(bool Add,UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerUpdatePointsOfInterests(bool Add, UMyManager_InteractionTarget* InteractionTarget);

	UFUNCTION(Reliable, Client)
	void ClientUpdatePointsOfInterests(bool Add, UMyManager_InteractionTarget* InteractionTarget);

	// Network
public:
	// UFUNCTION(Server,Reliable)
	// UFUNCTION(Client,Reliable)
	
	
	
protected:
	UFUNCTION()
	void OnInteractionTargetUpdatedServerSide(bool Add,UMyManager_InteractionTarget* InteractionTarget);
	
	UFUNCTION()
	void OnInteractionTargetUpdatedClientSide(bool Add,UMyManager_InteractionTarget* InteractionTarget);
	
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


	// Targets
protected:
	// UFUNCTION()
	// void OnNewTargetSelectedClientSide(UMyManager_InteractionTarget* NewTarget, bool IsSelected);
	//
	// UFUNCTION()
	// void SetTargetHighlighted(UMyManager_InteractionTarget* InteractionTarget, bool IsHighlighted);
	// Variables
	// Data
protected:

	
	

	
	// InteractionTarget 으로 교체하기?
	UPROPERTY()
	TArray<UMyManager_InteractionTarget*> InteractionTargets;

	UPROPERTY()
	TArray<UMyManager_InteractionTarget*> PointOfInterests;

	UPROPERTY()
	TArray<UMyManager_InteractionTarget*> PendingTargets;

	UPROPERTY()
	TArray<UMyManager_InteractionTarget*> DeactivatedTargets;
	
	UPROPERTY()
	TArray<UUW_InteractionTarget*> WidgetPool;

	UPROPERTY()
	TObjectPtr<UUW_InteractionTarget> CurrentInteractionMarker;

	UPROPERTY()
	TObjectPtr<UMyManager_InteractionTarget> BestInteractionTarget;
	
	UPROPERTY()
	TObjectPtr<APlayerController> OwnerController;

	UPROPERTY(BlueprintReadOnly,Category="Manager Interactor")
	TArray<FKey> InteractionKeys;
	
	UPROPERTY()
	FTimerHandle BeginUpdateKeysTimerHandle;

	UPROPERTY()
	TObjectPtr<UPostProcessComponent> PostProcessComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Outline_DynamicMaterial;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<UMaterialInterface> m_OutlineMaterial;

	// Main
protected:
	UPROPERTY()
	bool Debug = true;

	UPROPERTY()
	int32 DefaultWidgetPoolSize = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input",meta = (AllowPrivateAccess=true))
	TObjectPtr<UInputAction> InteractionInputAction;
	
};
