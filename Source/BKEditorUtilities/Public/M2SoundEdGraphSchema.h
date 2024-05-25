// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "SequencerData.h"
#include "M2SoundEdGraphSchema.generated.h"




UCLASS()
class BK_EDITORUTILITIES_API UM2SoundGraph : public UM2SoundGraphBase
{
	GENERATED_BODY()

public:

	UDAWSequencerData* GetSequencerData() const { return Cast<UDAWSequencerData>(GetOuter()); }

};

/**
 * 
 */
UCLASS()
class BK_EDITORUTILITIES_API UM2SoundEdGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()
	
	void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	
	
};

UCLASS()
class BK_EDITORUTILITIES_API UM2SoundEdGraphNode : public UEdGraphNode
{
	GENERATED_BODY()
public:
	bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override { return Schema->IsA<UM2SoundEdGraphSchema>(); }
	bool IncludeParentNodeContextMenu() const override { return true; }
	UM2SoundGraph* GetGraph() const { return Cast<UM2SoundGraph>(UEdGraphNode::GetGraph()); }
	UDAWSequencerData* GetSequencerData() const { return GetGraph()->GetSequencerData(); }

	FLinearColor GetNodeTitleColor() const override { return FColor(23, 23, 23, 23); }
	FLinearColor GetNodeBodyTintColor() const override { return FColor(220, 220, 220, 220); }
};