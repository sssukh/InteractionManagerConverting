// Fill out your copyright notice in the Description page of Project Settings.


#include "UW_InteractionTarget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

UUW_InteractionTarget::UUW_InteractionTarget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

void UUW_InteractionTarget::UpdateContentState(bool IsInteraction)
{
	ResetProgress();

	if(IsInteraction)
	{
		// if(Icon->Brush.GetResourceObject())
		// {
		// 	IsInteractionState = true;
		//
		// 	
		// }
	}
}
