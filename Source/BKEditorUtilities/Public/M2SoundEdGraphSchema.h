// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "M2SoundGraphData.h"

#include <EdGraphUtilities.h>
#include <EdGraph/EdGraphNode.h>
#include "SGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Vertexes/M2SoundVertex.h"

#include "M2SoundEdGraphSchema.generated.h"

class UM2SoundEdGraphNode;

 struct FPlacementDefaults
{
	 static const int OffsetBetweenNodes = 200;
	 static const int OutputsColumnPosition = 1400;
	 static const int InputsColumnPosition = 300;
	 static const int InstrumentColumns = 950;
};

UCLASS()
class BK_EDITORUTILITIES_API UM2SoundGraph : public UM2SoundGraphBase
{
	GENERATED_BODY()

public:

	// As currently it is impossible to access to the default ranges specified for input variables in the metasound patch
	// you need to set the ranges manually once for each node type and cache it, this method saves the ranges for all
	// currently selected patche vertices in the graph to the cache, the cache can be edited in the project settings
	UFUNCTION(CallInEditor, Category = "M2Sound")
	void SaveVertexRangesToCache();

	UDAWSequencerData* GetSequencerData() const { return Cast<UDAWSequencerData>(GetOuter()); }

	TArray<UEdGraphPin*> GetSelectedPins(EEdGraphPinDirection Direction) const;

	void AutoConnectTrackPinsForNodes(UM2SoundEdGraphNode& A, UM2SoundEdGraphNode& B);

	void InitializeGraph() override;

	template<class T>
	T* CreateDefaultNodeForVertex(UM2SoundVertex* Vertex, const int ColumnPosition)
	{
		FGraphNodeCreator<T> NodeCreator(*this);
		T* Node = NodeCreator.CreateNode();
		Node->Vertex = Vertex;
		Node->NodePosX = ColumnPosition;
		Node->NodePosY = Vertex->TrackId * FPlacementDefaults::OffsetBetweenNodes;

		NodeCreator.Finalize();

		//finally add node to map
		VertexToNodeMap.Add(Vertex, Node);

		return Node;
	}

	void PerformVertexToNodeBinding();

	//UPROPERTY(Transient, EditFixedSize, EditAnywhere, Instanced, meta = (TitleProperty = "{Name}") , Category = "Selection")
	TArray<UM2SoundEdGraphNode*> SelectedNodes;

	UPROPERTY(Transient, EditFixedSize, Instanced, EditAnywhere, Category = "Selection")
	TArray<UM2SoundVertex*> SelectedVertices;

	UFUNCTION()
	void OnVertexAdded(UM2SoundVertex* Vertex) { 
		
		//so this is definitely not good enough and indicates some issues
		// we need to think about the node creation process and how to handle the vertex to node mapping

		UE_LOG(LogTemp, Warning, TEXT("Vertex added"));
		NotifyGraphChanged(); 
	}



private:

	UPROPERTY()
	TMap<UM2SoundVertex*, UM2SoundEdGraphNode*> VertexToNodeMap;
};

/**
 *
 */
UCLASS()
class BK_EDITORUTILITIES_API UM2SoundEdGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()
public:

	const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;

	void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override
	{
		bool success = UEdGraphSchema::TryCreateConnection(A, B);
		if (success)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Connection created"));
			A->GetOwningNode()->NodeConnectionListChanged();
			B->GetOwningNode()->NodeConnectionListChanged();

			//node B check if is output node and call validate
		}

		return success;
	};

	//set pin type colors
	FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override
	{
		//Tracks are blue
		if (PinType.PinCategory == "Track-Audio")
		{
			return FLinearColor(0.0f, 0.0f, 1.0f);
		}

		//Audio is purple
		if (PinType.PinCategory == "Track-Midi")
		{
			return FLinearColor(0.5f, 0.0f, 0.5f);
		}

		//metasound literals get the value from the metasound literal schema according to their data type
		if (PinType.PinCategory == "MetasoundLiteral")
		{
			return FLinearColor::Red;
		}

		return UEdGraphSchema::GetPinTypeColor(PinType);
	};
};


USTRUCT()
struct FM2SoundGraphAddNodeAction : public FEdGraphSchemaAction
{
public:
	GENERATED_BODY()

	FM2SoundGraphAddNodeAction() : FEdGraphSchemaAction(), LocationOffset(FVector2D::ZeroVector) {}
	FM2SoundGraphAddNodeAction(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, const int32 InSectionID, const int32 InSortOrder);

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
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

USTRUCT()
struct FM2SoundGraphAddNodeAction_NewInstrument : public FM2SoundGraphAddNodeAction
{
	GENERATED_BODY()

	FM2SoundGraphAddNodeAction_NewInstrument() : FM2SoundGraphAddNodeAction(INVTEXT(""), INVTEXT("Instrument"), INVTEXT("Tooltip"), 0, 0, 0) {}
	UEdGraphNode* MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin) override;
};

USTRUCT()
struct FM2SoundGraphAddNodeAction_NewAudioOutput : public FM2SoundGraphAddNodeAction
{
	GENERATED_BODY()

	FM2SoundGraphAddNodeAction_NewAudioOutput() : FM2SoundGraphAddNodeAction(INVTEXT(""), INVTEXT("Audio Output"), INVTEXT("Tooltip"), 0, 0, 0) {}
	UEdGraphNode* MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin) override;
};

USTRUCT()
struct FM2SoundGraphAddNodeAction_NewAudioInsert : public FM2SoundGraphAddNodeAction
{
	GENERATED_BODY()

	FM2SoundGraphAddNodeAction_NewAudioInsert() : FM2SoundGraphAddNodeAction(INVTEXT(""), INVTEXT("Audio Insert"), INVTEXT("Tooltip"), 0, 0, 0) {}
	UEdGraphNode* MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin) override;
};

USTRUCT()
struct FM2SoundGraphAddNodeAction_NewGraphInputNode : public FM2SoundGraphAddNodeAction
{
	GENERATED_BODY()

	FM2SoundGraphAddNodeAction_NewGraphInputNode() : FM2SoundGraphAddNodeAction(INVTEXT("Inputs"), INVTEXT("Midi Track"), INVTEXT("Tooltip"), 0, 0, 0) {}
	UEdGraphNode* MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin) override;
};

USTRUCT()
struct FM2SoundGraphAddNodeAction_NewVariMixerNode : public FM2SoundGraphAddNodeAction
{
	GENERATED_BODY()

	FM2SoundGraphAddNodeAction_NewVariMixerNode() : FM2SoundGraphAddNodeAction(INVTEXT(""), INVTEXT("Mixer"), INVTEXT("A variable width mixer"), 0, 0, 0) {}
	UEdGraphNode* MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin) override;
};
