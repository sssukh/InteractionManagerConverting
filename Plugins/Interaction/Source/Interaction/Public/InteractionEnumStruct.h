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


USTRUCT(BlueprintType)
struct FInteractionBeginPayload
{
	GENERATED_BODY()

	FInteractionBeginPayload()
	{

	}

	FInteractionBeginPayload(APawn* InInteractorPawn)
	{
		InteractorPawn = InInteractorPawn;
	}

	UPROPERTY(BlueprintReadWrite, Category="Interaction Begin Payload")
	TObjectPtr<APawn> InteractorPawn;
};


// ServerOnInteractionUpdated 내부에서 SendEvent할 때 Payload에 사용할 구조체
USTRUCT(BlueprintType)
struct FInteractionUpdatePayload
{
	GENERATED_BODY()
	FInteractionUpdatePayload(): Alpha(0), Repeated(0)
	{
	}

	FInteractionUpdatePayload(float InAlpha,int32 InRepeated, APawn* InInteractorPawn)
	{
		Alpha = InAlpha;
		Repeated = InRepeated;
		InteractorPawn = InInteractorPawn;
	}
	UPROPERTY(BlueprintReadWrite, Category="Interaction Update Payload")
	float Alpha;

	UPROPERTY(BlueprintReadWrite, Category="Interaction Update Payload")
	int32 Repeated;

	UPROPERTY(BlueprintReadWrite, Category="Interaction Update Payload")
	TObjectPtr<APawn> InteractorPawn;
};



USTRUCT(BlueprintType)
struct FInteractionEndPayload
{
	GENERATED_BODY()

	FInteractionEndPayload(): Result()
	{
	}

	FInteractionEndPayload(EInteractionResult InResult, APawn* InInteractorPawn)
	{
		Result = InResult;

		InteractorPawn = InInteractorPawn;
	}

	UPROPERTY(BlueprintReadWrite, Category="Interaction EndPay load")
	EInteractionResult Result;

	UPROPERTY(BlueprintReadWrite, Category="Interaction EndPay load")
	TObjectPtr<APawn> InteractorPawn;
};