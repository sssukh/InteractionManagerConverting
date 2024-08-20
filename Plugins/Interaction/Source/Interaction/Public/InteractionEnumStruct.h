#pragma once

#include "CoreMinimal.h"
#include "InteractionEnumStruct.generated.h"

UENUM(BlueprintType)
enum class EInteractionResult : uint8
{
	Completed,
	Canceled,
	None
};

UENUM(BlueprintType)
enum class EInteractionNetworkHandleMethod : uint8
{
	KeepEnabled,
	DisableWhileInteracting
};

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	Tap,
	Hold,
	Repeat
};

UENUM(BlueprintType)
enum class EInteractionState : uint8
{
	None,
	Waiting,
	Interacting,
	Done
};

// UENUM(BlueprintType)
// enum class EInteractionFinishMethod : uint8
// {
// 	// 완료 또는 취소 시 파괴
// 	DestroyOnCompletion UMETA(DisplayName = "Destroy On Completion"),
//     
// 	// 완료 또는 취소 시 비활성화
// 	DeactivateOnCompletion UMETA(DisplayName = "Deactivate On Completion"),
//     
// 	// 완료 또는 취소 후 일정 시간 후에 다시 활성화
// 	ReactivateAfterDuration UMETA(DisplayName = "Reactivate After Duration"),
//     
// 	// 완료 시 파괴, 취소 시 비활성화
// 	DestroyOnCompletionDeactivateOnCancel UMETA(DisplayName = "Destroy On Completion, Deactivate On Cancel"),
//     
// 	// 완료 시 비활성화, 취소 시 파괴
// 	DeactivateOnCompletionDestroyOnCancel UMETA(DisplayName = "Deactivate On Completion, Destroy On Cancel")
// };

// /**
//  * EInteractionFinishMethod 열거형은 상호작용이 완료되거나 취소되었을 때의 후속 조치를 정의합니다.
//  * 각 값은 상호작용 완료 또는 취소 시 수행할 작업을 나타냅니다.
//  */
// UENUM(BlueprintType)
// enum class EInteractionFinishMethod : uint8
// {
// 	// 완료 시 파괴
// 	DestroyOnCompleted1 UMETA(DisplayName = "Destroy On Completed"),
// 	// 취소 시 파괴
// 	DestroyOnCanceled2 UMETA(DisplayName = "Destroy On Canceled"), 
// 	// 완료 또는 취소 시 파괴
// 	DestroyOnCompletedOrCanceled3 UMETA(DisplayName = "Destroy On Completed Or Canceled"), 
// 	// 완료 후 일정 시간 후에 다시 활성화
// 	ReactivateAfterDurationOnCompleted4 UMETA(DisplayName = "Reactivate After Duration On Completed"),
// 	// 취소 후 일정 시간 후에 다시 활성화
// 	ReactivateAfterDurationOnCanceled5 UMETA(DisplayName = "Reactivate After Duration On Canceled"), 
// 	// 완료 또는 취소 후 일정 시간 후에 다시 활성화
// 	ReactivateAfterDurationOnCompletedOrCanceled6 UMETA(DisplayName = "Reactivate After Duration On Completed Or Canceled"), 
// 	// 취소 후 일정 시간 후에 다시 활성화, 완료 시 파괴
// 	ReactivateAfterDurationOnCanceledDestroyOnCompleted7 UMETA(DisplayName = "Reactivate After Duration On Canceled, Destroy On Completed"), 
// 	// 취소 후 일정 시간 후에 다시 활성화, 완료 시 비활성화
// 	ReactivateAfterDurationOnCanceledDeactivateOnCompleted8 UMETA(DisplayName = "Reactivate After Duration On Canceled, Deactivate On Completed"),
// 	// 완료 후 일정 시간 후에 다시 활성화, 취소 시 파괴 
// 	ReactivateAfterDurationOnCompletedDestroyOnCanceled9 UMETA(DisplayName = "Reactivate After Duration On Completed, Destroy On Canceled"), 
// 	// 완료 후 일정 시간 후에 다시 활성화, 취소 시 비활성화
// 	ReactivateAfterDurationOnCompletedDeactivateOnCanceled10 UMETA(DisplayName = "Reactivate After Duration On Completed, Deactivate On Canceled"),
// 	// 완료 시 비활성화 
// 	DeactivateOnCompleted11 UMETA(DisplayName = "Deactivate On Completed"), 
// 	 // 취소 시 비활성화
// 	DeactivateOnCanceled12 UMETA(DisplayName = "Deactivate On Canceled"),
// 	// 완료 또는 취소 시 비활성화
// 	DeactivateOnCompletedOrCanceled13 UMETA(DisplayName = "Deactivate On Completed Or Canceled"), 
// 	// 취소 시 비활성화, 완료 시 파괴
// 	DeactivateOnCanceledDestroyOnCompleted14 UMETA(DisplayName = "Deactivate On Canceled, Destroy On Completed"), 
// 	// 취소 시 비활성화, 완료 후 일정 시간 후에 다시 활성화
// 	DeactivateOnCanceledReactivateAfterDurationOnCompleted15 UMETA(DisplayName = "Deactivate On Canceled, Reactivate After Duration On Completed"),
// 	// 완료 시 비활성화, 취소 시 파괴 
// 	DeactivateOnCompletedDestroyOnCanceled16 UMETA(DisplayName = "Deactivate On Completed, Destroy On Canceled"), 
// 	 // 완료 시 비활성화, 취소 후 일정 시간 후에 다시 활성화
// 	DeactivateOnCompletedReactivateAfterDurationOnCanceled17 UMETA(DisplayName = "Deactivate On Completed, Reactivate After Duration On Canceled")
// };


USTRUCT(BlueprintType)
struct FIconSelector
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Icon Selector")
	bool bUseMaterialAsTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Icon Selector")
	UTexture2D* Texture2D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Icon Selector")
	UMaterialInstance* Material;
};
