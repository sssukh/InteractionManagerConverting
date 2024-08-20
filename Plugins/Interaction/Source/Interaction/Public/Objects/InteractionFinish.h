#pragma once

#include "CoreMinimal.h"
#include "InteractionEnumStruct.h"
#include "UObject/Object.h"
#include "InteractionFinish.generated.h"

class UInteractionManager;
class UInteractionTarget;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew)
class INTERACTION_API UInteractionFinish : public UObject
{
	GENERATED_BODY()

public:
	void InitializeOnFinish(UInteractionManager* CallingInteractionManager, UInteractionTarget* InInteractionTarget);

	virtual void Execute(EInteractionResult InteractionResult);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Intercion Finish")
	bool bIsComplete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Intercion Finish")
	bool bIsCancel = false;

protected:
	UPROPERTY(BlueprintReadWrite, Category="Interaction Finish")
	AActor* InteractableActor;

	UPROPERTY(BlueprintReadWrite, Category="Interaction Finish")
	UInteractionTarget* InteractionTarget;

	UPROPERTY(BlueprintReadWrite, Category="Interaction Finish")
	UInteractionManager* InteractionManager;

	UPROPERTY(BlueprintReadWrite, Category="Interaction Finish")
	APawn* InteractionPawn;

	UPROPERTY(BlueprintReadWrite, Category="Interaction Finish")
	AController* InteractionController;
};
