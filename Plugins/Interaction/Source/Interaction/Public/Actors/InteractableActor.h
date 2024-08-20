#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interface_Interaction.h"
#include "InteractableActor.generated.h"

class UInteractionManager;
class UInteractionTarget;
class UStateTreeComponent;

UCLASS()
class INTERACTION_API AInteractableActor : public AActor, public IInterface_Interaction
{
	GENERATED_BODY()

public:
	AInteractableActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	void TryTakeAction(UInteractionManager* InteractingManager);

	UFUNCTION(BlueprintCallable, Category="Interactalbe Event")
	UInteractionManager* FindInteractingManager();

protected:
	void HandleTapInteraction(UInteractionManager* InteractingManager);
	void HandleHoldInteraction(UInteractionManager* InteractingManager);
	void HandleRepeatInteraction(UInteractionManager* InteractingManager);

	
	/*========================================================================================
	*	NetWork Event
	=========================================================================================*/
	UFUNCTION(Blueprintable, Server, Unreliable, Category="Manager Interactor|Network Event")
	void ServerOnInteractionBegin(UInteractionManager* InteractingManager);

	

	/*========================================================================================
	*	Interface_Action
	=========================================================================================*/
	// virtual void SendEvent_Implementation(FStateTreeEvent NewEvent) override;
	
	
	
	/*========================================================================================
	*	Field Members
	=========================================================================================*/
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactable Actor")
	USceneComponent* DefaultSceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactable Actor")
	USceneComponent* MarkerTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactable Actor")
	UStateTreeComponent* StateTreeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactable Actor")
	UInteractionTarget* InteractionTarget;

	UPROPERTY(BlueprintReadWrite, Category="Interactable Actor")
	bool bIsInteracting = false;

private:
	//현재 눌린시간
	float CurrentHoldTime;
	
	float RepeatCooldown;
	
	int32 Repeated;
};
