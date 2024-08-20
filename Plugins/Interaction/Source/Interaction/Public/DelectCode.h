// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DelectCode.generated.h"

/**
 * 
 */
UCLASS()
class INTERACTION_API UDelectCode : public UObject
{
	GENERATED_BODY()

#pragma region  Interaction Manager 삭제코드
	// void OnInteractionTargetReActivated(UInteractionTarget* InteractionTarget);

#pragma endregion

#pragma region Interaction Target 삭제코드
	//
	// // 상호작용 완료 후 처리 방법 설정
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|On Finished")
	// EInteractionFinishMethod FinishMethod = EInteractionFinishMethod::DestroyOnCompletion;
	//
	// // 상호작용 시작 델리게이트
	// UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Manager Interaction Target|Delegate")
	// FOnInteractionBegin OnInteractionBegin;
	//
	// // 상호작용 종료 델리게이트
	// UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Manager Interaction Target|Delegate")
	// FOnInteractionEnd OnInteractionEnd;
	//
	// // 상호작용 업데이트 델리게이트
	// UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Manager Interaction Target|Delegate")
	// FOnInteractionUpdated OnInteractionUpdated;
	//
	// // 상호작용 재활성화 델리게이트
	// UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Manager Interaction Target|Delegate")
	// FOnInteractionReactivated OnInteractionReactivated;
	//
	// // 상호작용 비활성화 델리게이트
	// UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Manager Interaction Target|Delegate")
	// FOnInteractionDeactivated OnInteractionDeactivated;

#pragma endregion
};
