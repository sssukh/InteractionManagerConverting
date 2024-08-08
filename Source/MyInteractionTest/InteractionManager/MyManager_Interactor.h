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
	
	// UFUNCTION()
	// void Debug_Function();

	//Interactable
protected:
	UFUNCTION()
	UMyManager_InteractionTarget* Find_Best_Interactable();

	
protected:
	UPROPERTY()
	TSubclassOf<UUW_InteractionTarget> InteractionWidgetClass;


	
	
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
	TObjectPtr<APlayerController> OwnerController;

	UPROPERTY()
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
	bool Debug = false;

	UPROPERTY()
	int32 DefaultWidgetPoolSize = 3;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UInputAction> InteractionInputAction;
	
};
