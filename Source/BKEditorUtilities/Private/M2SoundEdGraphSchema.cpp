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

UEdGraphNode* FM2SoundGraphAddNodeAction_NewOutput::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundGraphOutputNode> NodeCreator(*ParentGraph);
	UM2SoundGraphOutputNode* Node = NodeCreator.CreateUserInvokedNode();
	Node->Name = FName("Output");

	Node->Vertex = NewObject<UM2SoundOutput>(Node->GetSequencerData(),NAME_None, RF_Transactional);

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
