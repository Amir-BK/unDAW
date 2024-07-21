// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2AttachM2Vertex.generated.h"

/**
 * 
 */
UCLASS()
class UNDAWK2_NODES_API UK2AttachM2Vertex : public UK2Node
{
	GENERATED_BODY()

	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	FText GetTooltipText() const override;

	
};
