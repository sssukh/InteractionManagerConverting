#pragma once

#include "CoreMinimal.h"
#include "InteractionEnumStruct.h"
#include "StateTreeEvents.h"
#include "UObject/Interface.h"
#include "Interface_Interaction.generated.h"


UINTERFACE()
class UInterface_Interaction : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INTERACTION_API IInterface_Interaction
{
	GENERATED_BODY()

public:
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction System|Interface")
	// void SendEvent(FStateTreeEvent NewEvent);
		
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction System|Interface")
	// void OnInteractionBegin(APawn* NewInteractorPawn);
	//
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction System|Interface")
	// void OnInteractionUpdated(float InAlpha,int32 InInRepeated,APawn* InInteractorPawn);
	//
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction System|Interface")
	// void OnInteractionEnd(EInteractionResult InteractionResult, APawn* InteractionPawn);
	//
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction System|Interface")
	// void OnInteractionReactivated(APawn* ForPawn);
	//
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction System|Interface")
	// void OnInteractionDeactivated();
};
