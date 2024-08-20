// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionFinish.h"
#include "DestroyOnFinish.generated.h"

/**
 * 
 */
UCLASS()
class INTERACTION_API UDestroyOnFinish : public UInteractionFinish
{
	GENERATED_BODY()

public:
	virtual void Execute(EInteractionResult InteractionResult) override;
};
