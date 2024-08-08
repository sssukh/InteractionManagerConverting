// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyManager_InteractionTarget.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYINTERACTIONTEST_API UMyManager_InteractionTarget : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMyManager_InteractionTarget();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Checks
public:
	UFUNCTION()
	FORCEINLINE bool IsInteractionEnabled() {return InteractionEnabled;}

	// Variables
	// Data
public:
	UPROPERTY()
	bool InteractionEnabled = true;

	UPROPERTY()
	TObjectPtr<USceneComponent> MarkerTargetComponent;

	//Marker Settings
public:
	UPROPERTY()
	FVector MarkerOffset;
};
