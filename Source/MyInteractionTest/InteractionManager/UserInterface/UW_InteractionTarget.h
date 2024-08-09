// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyInteractionTest/InteractionManager/MyManager_InteractionTarget.h"
#include "UW_InteractionTarget.generated.h"

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
	UFUNCTION()
	void UpdateContentState(bool IsInteraction);
	
	UFUNCTION(BlueprintImplementableEvent,Category="UW Interaction Taget Event")
	void SetInteractionKeyText(FKey Key);

	UFUNCTION(BlueprintImplementableEvent,Category="UW Interaction Taget Event")
	void UpdateInteractionText(bool Immediately, Enum_InteractionState InteractionState);
	
	UFUNCTION(BlueprintImplementableEvent,Category = "UW Interaction Target Event")
	void ResetProgress();
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool IsInteractionState;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UTextBlock* LootKey;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UW Interaction Taget Event|Designer",meta=(BindWidget))
	UBorder* Icon;
	
	UPROPERTY()
	UMyManager_InteractionTarget* WidgetInteractionTarget;
};
