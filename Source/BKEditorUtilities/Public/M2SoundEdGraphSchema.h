// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "SequencerData.h"
#include <EdGraphUtilities.h>
#include <EdGraph/EdGraphNode.h>
#include "M2SoundEdGraphSchema.generated.h"




UCLASS()
class BK_EDITORUTILITIES_API UM2SoundGraph : public UM2SoundGraphBase
{
	GENERATED_BODY()

public:

	UDAWSequencerData* GetSequencerData() const { return Cast<UDAWSequencerData>(GetOuter()); }

	TArray<UEdGraphPin*> GetSelectedPins(EEdGraphPinDirection Direction) const;
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
	bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override { return Schema->IsA(UM2SoundEdGraphSchema::StaticClass()); }
	bool IncludeParentNodeContextMenu() const override { return true; }
	UM2SoundGraph* GetGraph() const { return Cast<UM2SoundGraph>(UEdGraphNode::GetGraph()); }
	UDAWSequencerData* GetSequencerData() const { return GetGraph()->GetSequencerData(); }

	FLinearColor GetNodeTitleColor() const override { return FColor(23, 23, 23, 23); }
	FLinearColor GetNodeBodyTintColor() const override { return FColor(220, 220, 220, 220); }


	UPROPERTY(EditAnywhere, Category = "Node")
	FName Name;

	UPROPERTY()
	UM2SoundVertex* Vertex;
};

UCLASS()
class UM2SoundGraphOutputNode : public UM2SoundEdGraphNode
{
	GENERATED_BODY()

public:
	void GetMenuEntries(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

};

UCLASS()
class UM2SoundGraphConsumer : public UM2SoundEdGraphNode
{
	GENERATED_BODY()
public:

	FText GetPinDisplayName(const UEdGraphPin* Pin) const override;
	//void AllocateDefaultPins() override;
	//void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	//void NodeConnectionListChanged() override;
	//void PostPasteNode() override;
	//void SyncModelConnections();
};


USTRUCT()
struct FM2SoundGraphAddNodeAction : public FEdGraphSchemaAction
{
public:	
	GENERATED_BODY()

	FM2SoundGraphAddNodeAction() : FEdGraphSchemaAction(), LocationOffset(FVector2D::ZeroVector) {}
	FM2SoundGraphAddNodeAction(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, const int32 InSectionID, const int32 InSortOrder);

	//virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual UEdGraphNode* MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin) PURE_VIRTUAL(FM2SoundGraphAddNodeAction::MakeNode, return nullptr;)


private:
	UPROPERTY()
	FText TransactionText;

	UPROPERTY()
	FVector2D LocationOffset;
};

USTRUCT()
struct FM2SoundGraphToOutputAction : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	FM2SoundGraphToOutputAction(const TArray<UEdGraphPin*>& InSourcePins);
	FM2SoundGraphToOutputAction() : FM2SoundGraphToOutputAction(TArray<UEdGraphPin*>()) {}
	//UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
private:
	TArray<UEdGraphPin*> SourcePins;
};

class FM2SoundGraphPanelNodeFactory : public FGraphPanelNodeFactory
{
	public:
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* InNode) const override;
};

USTRUCT()
struct FM2SoundGraphAddNodeAction_NewOutput : public FM2SoundGraphAddNodeAction
{
	GENERATED_BODY()

	FM2SoundGraphAddNodeAction_NewOutput() : FM2SoundGraphAddNodeAction(INVTEXT(""), INVTEXT("Output"), INVTEXT("Tooltip"), 0, 0, 0) {}
	UEdGraphNode* MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin) override;
};