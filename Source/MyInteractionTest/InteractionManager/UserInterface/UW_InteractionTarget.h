// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Layout/Margin.h"
#include "MyInteractionTest/InteractionManager/MyManager_InteractionTarget.h"
#include "UW_InteractionTarget.generated.h"

class UOverlay;
class UHorizontalBox;
class UGridPanel;

UENUM()
enum class Enum_InteractionState
{
	None,
	Waiting,
	Interacting,
	Done
};

class UTextBlock;
class UBorder;
class UWidgetSwitcher;
class USizeBox;
/**
 * 
 */
UCLASS()
class MYINTERACTIONTEST_API UUW_InteractionTarget : public UUserWidget
{
	GENERATED_BODY()
public:
	UUW_InteractionTarget(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintImplementableEvent,Category="UW Interaction Taget Event")
	void UpdateInteractionTarget(UMyManager_InteractionTarget* InteractionTarget);
	
	UFUNCTION(BlueprintImplementableEvent,Category="UW Interaction Taget Event")
	void UpdateInteractionText(bool Immediately, Enum_InteractionState InteractionState);
	
	UFUNCTION(BlueprintImplementableEvent,Category="UW Interaction Taget Event")
	void UpdateContentState(bool IsInteraction);
	
	UFUNCTION(BlueprintImplementableEvent,Category="UW Interaction Taget Event")
	void SetInteractionKeyText(FKey Key);

	
	UFUNCTION(BlueprintImplementableEvent,Category = "UW Interaction Target Event")
	void ResetProgress();
	
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Components")
	TObjectPtr<UMyManager_InteractionTarget> WidgetInteractionTarget;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector LastWorldLocation;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool IsInteractionState;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool ClearTarget;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	double CurrentPercent;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FMargin ScreenMargin;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	double ScreenRadiusPercent;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FText CurrenInteractionText;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector2D WidgetPositionErrorFactor;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool DisplayProgress;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	double CustomPercent;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool CustomClockwise = true;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UBorder* Background;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UWidgetSwitcher* ContentSwitcher;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UBorder* Foreground;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UBorder* Icon;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	USizeBox* IconContainer;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UTextBlock* InfoText;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UGridPanel* InteractionContainer;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UHorizontalBox* InteractionInfoBox;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UTextBlock* LootKey;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UBorder* LootKeyIcon;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UWidgetSwitcher* LootTextSwitcher;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	USizeBox* POI_Container;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UBorder* POI_Icon;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UBorder* Triangle;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UOverlay* TriangleBox;
};
