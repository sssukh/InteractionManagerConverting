#pragma once

UENUM()
enum class Enum_InteractionFinishMethod :uint32
{
	DestroyOnComplete,
	DestroyOnCanceled,
	DestroyOnCompletedOrCanceled,
	ReactivateAfterDurationOnCompleted,
	ReactivateAfterDurationOnCanceled,
	ReactivateAfterDurationOnCompletedOrCanceled,
	ReactivateAfterDurationOnCanceledDestroyOnCompleted,
	ReactivateAfterDurationOnCanceledDeactivateOnCompleted,
	ReactivateAfterDurationOnCompletedDestroyOnCanceled,
	ReactivateAfterDurationOnCompletedDeactivateOnCanceled,
	DeactivateOnCompleted,
	DeactivateOnCanceled,
	DeactivateOnCompletedOrCanceled,
	DeactivateOnCanceledDestroyOnCompleted,
	DeactivateOnCanceledReactivateAfterDurationOnCompleted,
	DeactivateOnCompletedDestroyOnCanceled,
	DeactivateOnCompletedReactivateAfterDurationOnCanceled,
};

UENUM(BlueprintType)
enum class Enum_InteractionNetworkHandleMethod : uint8
{
	KeepEnabled,
	DisableWhileInteracting,
};

UENUM(BlueprintType)
enum class Enum_InteractionResult : uint8
{
	Completed,
	Canceled,
};

UENUM(BlueprintType)
enum class Enum_InteractionType : uint8
{
	Tap,
	Hold,
	Repeat,
};

UENUM(BlueprintType)
enum class Enum_InteractionState : uint8
{
	None,
	Waiting,
	Interacting,
	Done,
};

struct FInteractionFinishMethod
{
	// 플래그 설정하는 것 확인해보기 
	int8 DestroyOnCompleted:1;
	int8 DestroyOnCanceled:1;

	int8 ReactivateAfterDurationOnCompleted:1;
	int8 ReactivateAfterDurationOnCanceled:1;

	int8 DeactivateOnCompleted:1;
	int8 DeactivateOnCanceled:1;
};