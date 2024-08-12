﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enums/InteractionManagerEnums.h"
#include "Util/ColorConstants.h"
#include "MyManager_InteractionTarget.generated.h"

class UMyManager_Interactor;



class USphereComponent;
struct FMargin;

USTRUCT(BlueprintType)
struct FStruct_IconSelector
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool UseMaterialAsTexture;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UTexture2D* Texture2D;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<UMaterialInstance> Material;
	
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractionBegin,/*For Pawn*/APawn*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInteractionEnd,/*Result*/Enum_InteractionResult,/*For Pawn*/APawn*);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnInteractionUpdated,/*Alpha*/double,/*Repeated*/int32,/*InteractorPawn*/APawn*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractionReactivated,/*For Pawn*/APawn*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractionDeactivated,/*For Pawn*/APawn*);


UCLASS(BlueprintType,Blueprintable,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))

class MYINTERACTIONTEST_API UMyManager_InteractionTarget : public UActorComponent
{
	GENERATED_BODY()

public:

	FOnInteractionBegin OnInteractionBegin;
	FOnInteractionEnd OnInteractionEnd;
	FOnInteractionUpdated OnInteractionUpdated;
	FOnInteractionReactivated OnInteractionReactivated;
	FOnInteractionDeactivated OnInteractionDeactivated;
	
	// Sets default values for this component's properties
	UMyManager_InteractionTarget();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	// Macro
public:
	UFUNCTION()
	UMyManager_Interactor* GetInteractorManager(AController* Controller);
	// Main
public:
	UFUNCTION()
	void ConstructOwnerEssentials();

	UFUNCTION()
	void ConstructOverlapZones();

	UFUNCTION()
	void ConstructHighlightedComponents();

	// Overlap Events
protected:
	UFUNCTION()
	void OnInnerZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInnerZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnOuterZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOuterZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	
	// Visual
public:
	UFUNCTION()
	void UpdateWidgetInfo(FMargin InWidgetMargin, double InScreenRadiusPercent);
	
	UFUNCTION()
	void SelectMarkerComponent(FString ComponentName);
	
	UFUNCTION()
	void SetHighlight(bool IsHighlighted);
	
	// Checks
public:
	UFUNCTION()
	FORCEINLINE bool IsInteractionEnabled() {return InteractionEnabled;}

	UFUNCTION()
	bool IsReactivationEnabled();

	UFUNCTION()
	bool CancelOnRelease();

	// Binded Actions
public:
	UFUNCTION()
	void OnDeactivated();

	UFUNCTION()
	void OnAddedToPendingTarget();
	
	UFUNCTION()
	void OnInteractionBeginEvent(APawn* InteractorPawn);

	UFUNCTION()
	void OnInteractionEndEvent(Enum_InteractionResult Result, APawn* InteractorPawn);
public:
	UFUNCTION()
	void AssignInteractor(bool Add,AController* Interactor);
	
	// Variables
	// Main
public:
	UPROPERTY(BlueprintReadWrite,Category = "InteractionTarget|Main|Components")
	TObjectPtr<USphereComponent> OuterZone;

	UPROPERTY(BlueprintReadWrite,Category = "InteractionTarget|Main|Components")
	TObjectPtr<USphereComponent> InnerZone;
	
	UPROPERTY(BlueprintReadOnly,Category = "InteractionTarget|Main|Components")
	TObjectPtr<AActor> OwnerReference;
	// Data
public:
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	double ScreenRadiusPercent;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	FMargin WidgetMargin;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	bool InteractionEnabled = true;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray<UPrimitiveComponent*> HighlightedComponents;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TObjectPtr<USceneComponent> MarkerTargetComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	double LastInteractedTime;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray<AController*> AssignedInteractors; 
	//Marker Settings
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	FString MarkerComponentName;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	FVector MarkerOffset;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	bool DisplayProgress = true;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	bool DisplayTriangle = true;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	bool FlashPOI_Icon;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	bool Flash_TargetIcon;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	bool Rotate_InteractionIcon;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	FLinearColor Target_IconColor;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	FLinearColor POI_IconColor;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	FLinearColor Interaction_IConBackground_Color;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	FLinearColor Interaction_IconForeground_Color;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	FStruct_IconSelector Target_ICon;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	FStruct_IconSelector POI_ICon;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	FStruct_IconSelector Interaction_Icon_Background;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Marker Settings")
	FStruct_IconSelector Interaction_Icon_Foreground;
	
	
	// Highlight Settings
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Highlight Settings")
	TArray<FString> ComponentsToHighlight;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Highlight Settings")
	FLinearColor HighlightColor;

	// Interaction Settings
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Interaction Settings")
	TMap<Enum_InteractionState,FText> InteractionText; 
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Interaction Settings")
	FString InteractionZone_ComponentToAttach;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Interaction Settings")
	double InnerZoneRadius;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Interaction Settings")
	double OuterZoneExtent;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Interaction Settings")
	Enum_InteractionType InteractionType;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Interaction Settings")
	Enum_InteractionNetworkHandleMethod NetworkHandleMethod;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Interaction Settings")
	double HoldSeconds;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Interaction Settings")
	int32 RepeatCount;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Interaction Settings")
	double RepeatCooldown;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Interaction Settings")
	bool CooldownEnabled;
	// Key Settings
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Key Settings")
	bool DisplayInteractionKey;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Key Settings")
	bool UseCustomKeys;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|Key Settings")
	TArray<FKey> CustomKeys;

	// On Finished
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|On Finished")
	Enum_InteractionFinishMethod FinishMethod;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget|On Finished")
	double ReactivationDuration;
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractionTarget")
	bool EnableDebug = true;
};
