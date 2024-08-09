// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Util/ColorConstants.h"
#include "MyManager_InteractionTarget.generated.h"

class USphereComponent;

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

	// Main
public:
	UFUNCTION()
	void ConstructOwnerEssentials();

	UFUNCTION()
	void ConstructOverlapZones();

	UFUNCTION()
	void ConstructHighlightedComponents();

	// Overlap Events
protected:
	UFUNCTION()
	void OnInnerZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInnerZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnOuterZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOuterZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	
	// Visual
public:
	UFUNCTION()
	void SelectMarkerComponent(FString ComponentName);
	
	UFUNCTION()
	void SetHighlight(bool IsHighlighted);
	
	// Checks
public:
	UFUNCTION()
	FORCEINLINE bool IsInteractionEnabled() {return InteractionEnabled;}

	// Variables
	// Main
public:
	UPROPERTY(BlueprintReadWrite,Category = "InteractionTarget|Main|Components")
	TObjectPtr<USphereComponent> OuterZone;

	UPROPERTY(BlueprintReadWrite,Category = "InteractionTarget|Main|Components")
	TObjectPtr<USphereComponent> InnerZone;
	
	UPROPERTY(BlueprintReadOnly,Category = "InteractionTarget|Main")
	TObjectPtr<AActor> OwnerReference;
	// Data
public:
	UPROPERTY()
	bool InteractionEnabled = true;

	UPROPERTY()
	TArray<UPrimitiveComponent*> HighlightedComponents;
	
	UPROPERTY()
	TObjectPtr<USceneComponent> MarkerTargetComponent;

	//Marker Settings
public:
	UPROPERTY()
	FVector MarkerOffset;

	// Highlight Settings
public:
	UPROPERTY()
	FString MarkerComponentName;
	
	UPROPERTY()
	FLinearColor HighlightColor;

	// Interaction Settings
public:
	UPROPERTY()
	FString InteractionZone_ComponentToAttach;
	
	// Key Settings
public:
	UPROPERTY()
	bool DisplayInteractionKey;
	
	UPROPERTY()
	bool UseCustomKeys;

	UPROPERTY()
	TArray<FKey> CustomKeys;

public:
	UPROPERTY()
	bool EnableDebug = true;
};
