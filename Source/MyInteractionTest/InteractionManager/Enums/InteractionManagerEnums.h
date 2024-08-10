#pragma once

UENUM()
enum class Enum_InteractionFinishMethod
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

UENUM()
enum class Enum_InteractionNetworkHandleMethod
{
	KeepEnabled,
	DisableWhileInteracting,
};

UENUM()
enum class Enum_InteractionResult
{
	Completed,
	Canceled,
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