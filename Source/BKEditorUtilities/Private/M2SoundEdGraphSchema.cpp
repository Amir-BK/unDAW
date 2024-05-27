// Fill out your copyright notice in the Description page of Project Settings.


#include "M2SoundEdGraphSchema.h"


void UM2SoundEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	UEdGraphSchema::GetGraphContextActions(ContextMenuBuilder);

	if (!ContextMenuBuilder.FromPin)
	{
		//const TArray<UEdGraphPin*> TargetPins = Cast<UConcordModelGraph>(ContextMenuBuilder.CurrentGraph)->GetSelectedPins(EGPD_Input);
		//if (!TargetPins.IsEmpty()) ContextMenuBuilder.AddAction(MakeShared<FConcordModelGraphFromParameterAction>(TargetPins));

		//const TArray<UEdGraphPin*> SourcePins = Cast<UM2SoundGraph>(ContextMenuBuilder.CurrentGraph)->GetSelectedPins(EGPD_Output);
		//if (!SourcePins.IsEmpty()) 
			ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewOutput>());
	}
}

FM2SoundGraphAddNodeAction::FM2SoundGraphAddNodeAction(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, const int32 InSectionID, const int32 InSortOrder)
	: FEdGraphSchemaAction(InNodeCategory, INVTEXT("Output"), InToolTip, 0)
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


void UM2SoundGraphOutputNode::GetMenuEntries(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	if (ContextMenuBuilder.FromPin &&
		ContextMenuBuilder.FromPin->Direction != EGPD_Output) return;
	ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction>());
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

void UM2SoundGraph::InitializeGraph()
{
	//if (GetSequencerData()) return;
	//SetSequencerData(NewObject<UM2SoundData>(

	//retrieve all output vertices from the sequencer data and create a node for each one
	//for (UM2SoundOutput* Output : GetSequencerData()->G)

	Nodes.Empty();

	const int OffsetBetweenNodes = 50;
	const int OutputsColumnPosition = 800;

	int i = 0;

	for(const auto& [name, Output] : GetSequencerData()->Outputs)
	{
		FGraphNodeCreator<UM2SoundGraphOutputNode> NodeCreator(*this);
		UM2SoundGraphOutputNode* Node = NodeCreator.CreateNode();
		UE_LOG(LogTemp, Log, TEXT("Creating node for output %s"), *name.ToString());
		Node->Vertex = Output;
		Node->Name = name;
		//Node->GetNodeTitle(ENodeTitleType::FullTitle).SetUseLargeFont(true);
		Node->NodePosX = OutputsColumnPosition;
		Node->NodePosY = i * OffsetBetweenNodes;
		NodeCreator.Finalize();
		i++;
	}

}

UEdGraphNode* FM2SoundGraphAddNodeAction_NewOutput::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundGraphOutputNode> NodeCreator(*ParentGraph);
	UM2SoundGraphOutputNode* Node = NodeCreator.CreateUserInvokedNode();
	Node->Name = FName("Output");

	Node->Vertex = NewObject<UM2SoundOutput>(Node->GetSequencerData(),NAME_None, RF_Transactional);
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
