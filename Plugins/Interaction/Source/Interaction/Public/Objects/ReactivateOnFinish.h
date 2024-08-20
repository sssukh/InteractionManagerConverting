// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionFinish.h"
#include "ReactivateOnFinish.generated.h"

/**
 * 
 */
UCLASS()
class INTERACTION_API UReactivateOnFinish : public UInteractionFinish
{
	GENERATED_BODY()

public:
	virtual void Execute(EInteractionResult InteractionResult) override;

protected:
	UFUNCTION()
	void CheckForPendingTargets();

	/*========================================================================================
	*	Field Members
	=========================================================================================*/
	
	/*재활성화 시간(초)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reactivate On Finish|Config")
	float ReActivationDuration = 5.0f;

	// 보류 중인 타겟을 확인하는 간격입니다. 0.1 ~ 1.0 사이의 값으로 조정할 수 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reactivate On Finish|Config", meta=(ClampMin="0.1", ClampMax="1.0", UIMin="0.1", UIMax="1.0"))
	float PendingTargetCheckInterval = 0.25f;

private:
	// 보류 중인 타겟을 확인하기 위한 타이머 핸들입니다.
	UPROPERTY(BlueprintReadWrite, Category="Reactivate On Finish", meta=(AllowPrivateAccess=true))
	FTimerHandle PendingTarget_TimerHandle;
};
