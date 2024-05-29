// Fill out your copyright notice in the Description page of Project Settings.


#include "M2SoundEdGraphSchema.h"


const FPinConnectionResponse UM2SoundEdGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (A->Direction == B->Direction)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot connect output to output or input to input."));
	
	if (A->PinType.PinCategory == "Track" && B->PinType.PinCategory == "Track")
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT("Connect Track Bus"));
	}
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not implemented by this schema."));
}

void UM2SoundEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	UEdGraphSchema::GetGraphContextActions(ContextMenuBuilder);

	if (!ContextMenuBuilder.FromPin)
	{
		ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewOutput>());
		ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewInstrument>());
	}
}

FM2SoundGraphAddNodeAction::FM2SoundGraphAddNodeAction(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, const int32 InSectionID, const int32 InSortOrder)
	: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, 0)
{

}

UEdGraphNode* FM2SoundGraphAddNodeAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	const FScopedTransaction Transaction(TransactionText);
	ParentGraph->Modify();
	Cast<UM2SoundGraph>(ParentGraph)->GetSequencerData()->Modify();
	UEdGraphNode* NewNode = MakeNode(ParentGraph, FromPin);
	NewNode->NodePosX = Location.X + LocationOffset.X;
	NewNode->NodePosY = Location.Y + LocationOffset.Y;
	if (FromPin) for (UEdGraphPin* Pin : NewNode->Pins)
	{
		if (FromPin->Direction == Pin->Direction) continue;
		ParentGraph->GetSchema()->TryCreateConnection(Pin, FromPin);
		NewNode->NodeConnectionListChanged();
		FromPin->GetOwningNode()->NodeConnectionListChanged();
		break;
	}
	return NewNode;
}


void UM2SoundGraphOutputNode::AllocateDefaultPins()
{
	// Create any pin for testing
	CreatePin(EGPD_Input, "Track", FName("Track", 0));
	Pins.Last()->DefaultValue = "Default";
}

void UM2SoundGraphOutputNode::GetMenuEntries(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	if (ContextMenuBuilder.FromPin &&
		ContextMenuBuilder.FromPin->Direction != EGPD_Output) return;
	//ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction>());
}

TSharedPtr<class SGraphNode> FM2SoundGraphPanelNodeFactory::CreateNode(UEdGraphNode* InNode) const
{
	return nullptr;
}

FM2SoundGraphToOutputAction::FM2SoundGraphToOutputAction(const TArray<UEdGraphPin*>& InSourcePins) : FEdGraphSchemaAction(INVTEXT("Meta"), INVTEXT("To Output"), INVTEXT("Creates a M2Sound output for each selected output pin."), 0)
, SourcePins(InSourcePins)
{}

TArray<UEdGraphPin*> UM2SoundGraph::GetSelectedPins(EEdGraphPinDirection Direction) const
{
	TArray<UEdGraphPin*> Pins;

	return MoveTemp(Pins);
}

void UM2SoundGraph::AutoConnectTrackPinsForNodes(const UM2SoundEdGraphNode& A, const UM2SoundEdGraphNode& B)
{

	UEdGraphPin* AOutputPin = nullptr;
	
	for(UEdGraphPin* Pin : A.Pins)
	{
		if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == "Track")
		{
			AOutputPin = Pin;
		}
	}

	if(!AOutputPin) return;

	for(UEdGraphPin* Pin : B.Pins)
	{
		if (Pin->Direction == EGPD_Input && Pin->PinType.PinCategory == "Track")
		{
			 GetSchema()->TryCreateConnection(AOutputPin, Pin);
		}
	}


	
}

void UM2SoundGraph::InitializeGraph()
{
	Nodes.Empty();

	const int OffsetBetweenNodes = 75;
	const int OutputsColumnPosition = 1400;
	const int InputsColumnPosition = 600;
	const int InstrumentColumns = 950;

	int i = 0;


	for(const auto& [index, Track] : GetSequencerData()->TrackInputs)
	{
		FGraphNodeCreator<UM2SoundGraphInputNode> NodeCreator(*this);
		UM2SoundGraphInputNode* Node = NodeCreator.CreateNode();

		Node->Vertex = Track;
		Node->TrackId = index - 1;
		Node->AssignedTrackId = index - 1;
		Node->Name = FName(GetSequencerData()->GetTracksDisplayOptions(Node->TrackId).trackName);
		//Node->GetNodeTitle(ENodeTitleType::FullTitle).SetUseLargeFont(true);
		Node->NodePosX = InputsColumnPosition;
		Node->NodePosY = i * OffsetBetweenNodes;
		NodeCreator.Finalize();

		FGraphNodeCreator<UM2SoundInstrumentNode> InstrumentNodeCreator(*this);
		UM2SoundInstrumentNode* InstrumentNode = InstrumentNodeCreator.CreateNode();

		auto& InstrumentVertex = Track->Outputs[0];

		InstrumentNode->Vertex = InstrumentVertex;
		InstrumentNode->NodePosX = InstrumentColumns;
		InstrumentNode->NodePosY = i * OffsetBetweenNodes;
		
		InstrumentNodeCreator.Finalize();

		FGraphNodeCreator<UM2SoundGraphOutputNode> OutputNodeCreator(*this);
		UM2SoundGraphOutputNode* OutputNode = OutputNodeCreator.CreateNode();

		auto& OutputVertex = InstrumentVertex->Outputs[0];

		OutputNode->Vertex = OutputVertex;
		OutputNode->NodePosX = OutputsColumnPosition;
		OutputNode->NodePosY = i * OffsetBetweenNodes;

		OutputNodeCreator.Finalize();

		AutoConnectTrackPinsForNodes(*Node, *InstrumentNode);
		AutoConnectTrackPinsForNodes(*InstrumentNode, *OutputNode);

		i++;
	}

}

UEdGraphNode* FM2SoundGraphAddNodeAction_NewOutput::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundGraphOutputNode> NodeCreator(*ParentGraph);
	UM2SoundGraphOutputNode* Node = NodeCreator.CreateUserInvokedNode();
	Node->Name = FName("Output");

	Node->Vertex = NewObject<UM2SoundMidiOutput>(Node->GetSequencerData(),NAME_None, RF_Transactional);
	Node->GetSequencerData()->AddVertex(Node->Vertex);
	NodeCreator.Finalize();

	
	return Node;
}

FText UM2SoundGraphConsumer::GetPinDisplayName(const UEdGraphPin* Pin) const
{
	if (Pin->Direction == EGPD_Output) return FText::FromName(Pin->PinName);
	int32 Index = Pins.Find(const_cast<UEdGraphPin*>(Pin));
	check(Index != INDEX_NONE && Vertex);
	//if (Index >= Vertex->GetInputInfo().Num()) return INVTEXT("This pin should not exist! Remove and re-add the node.");
	//return Vertex->GetInputInfo()[Index].DisplayName;

	return INVTEXT("Le Poop");
}

void UM2SoundGraphInputNode::AllocateDefaultPins()
{
		// Create any pin for testing
	CreatePin(EGPD_Output, "Track", FName("Track", 0));
	Pins.Last()->DefaultValue = "Default";
}

void UM2SoundInstrumentNode::AllocateDefaultPins()
{
	// Create any pin for testing
	CreatePin(EGPD_Input, "Track", FName("Track", 0));
	Pins.Last()->DefaultValue = "Default";

	CreatePin(EGPD_Output, "Track", FName("Track", 0));
	Pins.Last()->DefaultValue = "Default";
}

UEdGraphNode* FM2SoundGraphAddNodeAction_NewInstrument::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundInstrumentNode> NodeCreator(*ParentGraph);
	UM2SoundInstrumentNode* Node = NodeCreator.CreateUserInvokedNode();
	Node->Name = FName("Instrument");
	NodeCreator.Finalize();

	Node->Vertex = NewObject<UM2SoundPatch>(Node->GetSequencerData(), NAME_None, RF_Transactional);
	return Node;
}
