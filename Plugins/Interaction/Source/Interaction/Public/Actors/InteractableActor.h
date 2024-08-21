#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interface_Interaction.h"
#include "InteractableActor.generated.h"

class UUW_InteractionTarget;
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
	UFUNCTION(BlueprintCallable, Category="Interactalbe Event")
	void TryTakeAction(UInteractionManager* InteractingManager);

	UFUNCTION(BlueprintCallable, Category="Interactalbe Event")
	UInteractionManager* FindInteractingManager();

protected:
	void HandleTapInteraction(UInteractionManager* InteractingManager);

	void HandleHoldInteraction(UInteractionManager* InteractingManager);

	void HandleRepeatInteraction(UInteractionManager* InteractingManager);

public:
	UFUNCTION(BlueprintCallable, Category="Interactalbe Actor|Interaction")
	void OnInteractionUpdated(UInteractionManager* InteractingManager, float InAlpha, int32 InRepeated);

	/*========================================================================================
	*	NetWork Event
	=========================================================================================*/
public:
	UFUNCTION(Blueprintable, Server, Unreliable, Category="Interactalbe Actor|Network Event")
	void ServerOnInteractionBegin(UInteractionManager* InteractingManager);

	UFUNCTION(Blueprintable, Server, Unreliable, Category="Manager Interactor|Network Event")
	void ServerOnInteractionUpdated(UInteractionManager* InteractingManager, float InAlpha, int32 InRepeated, APawn* InteractionPawn);

	UFUNCTION(Blueprintable, Server, Unreliable, Category="Interactalbe Actor|Network Event")
	void ServerOnInteractionFinished(UInteractionManager* InteractingManager, EInteractionResult InteractionResult);

	UFUNCTION(Blueprintable, Client, Unreliable, Category="Interactalbe Actor|Network Event")
	void ClientResetData();

	UFUNCTION(Blueprintable, Client, Unreliable, Category="Interactalbe Actor|Network Event")
	void ClientCheckPressedKey(UInteractionManager* InteractingManager);


	/*========================================================================================
	*	Interface_Action
	=========================================================================================*/
public:
	virtual void SetEnableInteractivity_Implementation(bool bIsEnable) override;
	virtual void ResetData_Implementation() override;
	virtual void SendEvent_Implementation(FStateTreeEvent NewEvent) override;


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
	UPROPERTY(BlueprintReadOnly, Category="Interactable Actor", meta=(AllowPrivateAccess=true))
	bool bKeyJustPressed;

	UPROPERTY(BlueprintReadOnly, Category="Interactable Actor", meta=(AllowPrivateAccess=true))
	FKey LastPressedKey;

public:
	//현재 눌린시간
	UPROPERTY(BlueprintReadWrite, Category="Interactable Actor", meta=(AllowPrivateAccess=true))
	float CurrentHoldTime;

	UPROPERTY(BlueprintReadWrite, Category="Interactable Actor", meta=(AllowPrivateAccess=true))
	float RepeatCooldown;

	UPROPERTY(BlueprintReadWrite, Category="Interactable Actor", meta=(AllowPrivateAccess=true))
	int32 Repeated;

	UPROPERTY(BlueprintReadWrite, Category="Interactable Actor", meta=(AllowPrivateAccess=true))
	UUW_InteractionTarget* OwnedInteractionWidget;
};
