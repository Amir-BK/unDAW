// Fill out your copyright notice in the Description page of Project Settings.


#include "M2SoundEdGraphSchema.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"
#include "AudioParameter.h"
#include "IAudioParameterInterfaceRegistry.h"
#include "Sound/SoundBase.h"


const FPinConnectionResponse UM2SoundEdGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (A->Direction == B->Direction)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot connect output to output or input to input."));
	
	if (A->PinType.PinCategory == "Track" && B->PinType.PinCategory == "Track")
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, TEXT("Connect Track Bus"));
	}
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not implemented by this schema."));
}

void UM2SoundEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	UEdGraphSchema::GetGraphContextActions(ContextMenuBuilder);

	if(auto& FromPin = ContextMenuBuilder.FromPin)
	{
		if (FromPin->PinType.PinCategory == "Track")
		{
			ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewInstrument>());
			ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewAudioOutput>());
			ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewAudioInsert>());
		}
	}

	if (!ContextMenuBuilder.FromPin)
	{
		ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewOutput>());
		ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewInstrument>());
		ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewAudioOutput>());
		ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewAudioInsert>());
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

	//NewNode->bHasCompilerMessage = true;
	//NewNode->GetDeprecationResponse()->Message = FText::FromString("This node is deprecated and will be removed in a future version of the plugin.");

	NewNode->GetGraph()->NotifyGraphChanged();

	return NewNode;
}


void UM2SoundGraphMidiOutputNode::AllocateDefaultPins()
{
	// Create any pin for testing
	CreatePin(EGPD_Input, "Track", FName("Track", 0));
	Pins.Last()->DefaultValue = "Default";
}

void UM2SoundGraphMidiOutputNode::GetMenuEntries(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	if (ContextMenuBuilder.FromPin &&
		ContextMenuBuilder.FromPin->Direction != EGPD_Output) return;
	//ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction>());
}

void UM2SoundGraphMidiOutputNode::NodeConnectionListChanged()
{

	if (!Pins[0]->HasAnyConnections())
	{

		bHasCompilerMessage = true;
		ErrorType = EMessageSeverity::Info;
		ErrorMsg = FString("MIDI output vertex has no inputs");
		bHasChanges = true;
	}
	else {
		ClearCompilerMessage();
		ErrorMsg = FString("");
		bHasChanges = true;
	}

	UM2SoundEdGraphNodeConsumer::NodeConnectionListChanged();

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
			bool bSuccess = GetSchema()->TryCreateConnection(AOutputPin, Pin);
			if(bSuccess)
			{
				//A.NodeConnectionListChanged();
				//B.NodeConnectionListChanged();
			}
		}
	}


	
}

void UM2SoundGraph::InitializeGraph()
{
	Nodes.Empty();

	const int OffsetBetweenNodes = 200;
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

		FGraphNodeCreator<UM2SoundGraphMidiOutputNode> OutputNodeCreator(*this);
		UM2SoundGraphMidiOutputNode* OutputNode = OutputNodeCreator.CreateNode();

		auto& OutputVertex = InstrumentVertex->Outputs[0];

		OutputNode->Vertex = OutputVertex;
		OutputNode->NodePosX = OutputsColumnPosition;
		OutputNode->NodePosY = i * OffsetBetweenNodes;

		OutputNodeCreator.Finalize();

		AutoConnectTrackPinsForNodes(*Node, *InstrumentNode);
		AutoConnectTrackPinsForNodes(*InstrumentNode, *OutputNode);

		Node->VertexUpdated();
		InstrumentNode->VertexUpdated();
		OutputNode->VertexUpdated();


		i++;
	}

	PerformVertexToNodeBinding();
	NotifyGraphChanged();

	GetSequencerData()->OnVertexAdded.AddUniqueDynamic(this, &UM2SoundGraph::OnVertexAdded);
}

void UM2SoundGraph::PerformVertexToNodeBinding()
{
	for (const auto NodeAsObject : Nodes)
	{
		auto Node = Cast<UM2SoundEdGraphNode>(NodeAsObject);
		if(!Node) continue;

		UM2SoundVertex* Vertex = Node->Vertex;
		if (!Vertex) continue;

		Vertex->OnVertexUpdated.AddUniqueDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
	}

}

UEdGraphNode* FM2SoundGraphAddNodeAction_NewOutput::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundGraphMidiOutputNode> NodeCreator(*ParentGraph);
	UM2SoundGraphMidiOutputNode* Node = NodeCreator.CreateUserInvokedNode();
	Node->Name = FName("Output");

	Node->Vertex = NewObject<UM2SoundMidiOutput>(Node->GetSequencerData(),NAME_None, RF_Transactional);
	Node->Vertex->OnVertexUpdated.AddDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
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

	//Node->Err

	Node->Vertex = NewObject<UM2SoundPatch>(Node->GetSequencerData(), NAME_None, RF_Transactional);
	Node->Vertex->OnVertexUpdated.AddDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
	Node->GetSequencerData()->AddVertex(Node->Vertex);
	return Node;
}

void UM2SoundEdGraphNode::NodeConnectionListChanged()
{
	UEdGraphNode::NodeConnectionListChanged();

	//update CurrentTrackOuputs
	CurrentTrackOutputs.Empty();
	for (auto Pin : Pins)
	{
		if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == "Track")
		{
			//if Pin has connections, cast them to M2SoundEdGraphNode and add them to CurrentTrackOutputs
			if (Pin->LinkedTo.Num() > 0)
			{
				for (auto LinkedPin : Pin->LinkedTo)
				{
					if (LinkedPin->GetOwningNode()->IsA<UM2SoundEdGraphNode>())
					{
						CurrentTrackOutputs.Add(Cast<UM2SoundEdGraphNode>(LinkedPin->GetOwningNode()));
					}
				}

			}
		}

		GetGraph()->NotifyGraphChanged();

	}
}

void UM2SoundEdGraphNode::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	//does this ever get called?

	UE_LOG(LogTemp, Warning, TEXT("ValidateNodeDuringCompilation"));


}

void UM2SoundEdGraphNode::VertexUpdated()
{
	UE_LOG(LogTemp, Warning, TEXT("VertexUpdated"));


	//assuming each vertex only implements one interface, we can keep most of the logic here
	Audio::FParameterInterfacePtr interface = nullptr;
	if(IsA<UM2SoundInstrumentNode>())
	{
		UE_LOG(LogTemp, Warning, TEXT("IsInstrumentNode"));
		interface = unDAW::Metasounds::FunDAWInstrumentRendererInterface::GetInterface();

	}

	if (IsA<UM2SoundAudioInsert>() || IsA<UM2SoundInstrumentNode>())
	{
		auto AsPatch = Cast<UM2SoundPatch>(Vertex);
		//if patch is valid, get its name and give it to this node
		if(AsPatch->Patch)
		{
			Name = AsPatch->Patch->GetDocumentChecked().Metadata.Version.Name;
		}
		else
		{
			Name = "No Patch Selected";
		}
	}
	if(Vertex->bHasErrors)
	{
		bHasCompilerMessage = Vertex->bHasErrors;
		ErrorType = Vertex->ErrorType;
		ErrorMsg = Vertex->VertexErrors;
	}
	else
	{
		bHasCompilerMessage = false;
		UE_LOG(LogTemp, Warning, TEXT("VertexUpdated message should have been cleared!"));
		ErrorMsg = FString("");
		ClearCompilerMessage();
	}

	//clear autogen pins
	for(auto& Pin : AutoGeneratedPins)
	{
		Pin->BreakAllPinLinks();
		RemovePin(Pin);
	}

	AutoGeneratedPins.Empty();

	//create pins for meta sound ios
	for(auto& VertexMetasoundOutput : Vertex->MetasoundOutputs)
	{
		bool bBelongsToInterface = false;

		if(interface)
		{
			TArray<Audio::FParameterInterface::FOutput> FoundMatchingOutputs =
				interface->GetOutputs().FilterByPredicate([&VertexMetasoundOutput](const Audio::FParameterInterface::FOutput& Output)
				{
					return Output.ParamName == VertexMetasoundOutput.PinName;
				});

			if(FoundMatchingOutputs.Num() > 0)
			{
				bBelongsToInterface = true;
			}
		}
		if(!bBelongsToInterface)
		{
			CreatePin(EGPD_Output, VertexMetasoundOutput.DataType, VertexMetasoundOutput.PinName);
			//Pins.Last()->DefaultValue = VertexMetasoundOutput.Key;
			AutoGeneratedPins.Add(Pins.Last());
		}
		

	}

	for(auto& VertexMetasoundInput : Vertex->MetasoundInputs)
	{
		bool bBelongsToInterface = false;

		if(interface)
		{
			TArray<Audio::FParameterInterface::FInput> FoundMatchingInputs =
				interface->GetInputs().FilterByPredicate([&VertexMetasoundInput](const Audio::FParameterInterface::FInput& Input)
				{
					return Input.InitValue.ParamName == VertexMetasoundInput.PinName;
				});

			if(FoundMatchingInputs.Num() > 0)
			{
				bBelongsToInterface = true;
			}
		}
		if(!bBelongsToInterface)
		{
			CreatePin(EGPD_Input, VertexMetasoundInput.DataType, VertexMetasoundInput.PinName);
			//Pins.Last()->DefaultValue = VertexMetasoundInput.Key;
			AutoGeneratedPins.Add(Pins.Last());
		}

		
	}

	GetGraph()->NotifyGraphChanged();
	
}

void UM2SoundGraphAudioOutputNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "Track", FName("Track", 0));
	Pins.Last()->DefaultValue = "Default";
}

UEdGraphNode* FM2SoundGraphAddNodeAction_NewAudioOutput::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundGraphAudioOutputNode> NodeCreator(*ParentGraph);
	UM2SoundGraphAudioOutputNode* Node = NodeCreator.CreateUserInvokedNode();
	Node->Name = FName("Audio Output");

	Node->Vertex = NewObject<UM2SoundAudioOutput>(Node->GetSequencerData(), NAME_None, RF_Transactional);
	Node->GetSequencerData()->AddVertex(Node->Vertex);
	NodeCreator.Finalize();

	return Node;
}

void UM2SoundAudioInsertNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "Track", FName("Track", 0));
	Pins.Last()->DefaultValue = "Default";

	CreatePin(EGPD_Output, "Track", FName("Track", 0));
	Pins.Last()->DefaultValue = "Default";
}

UEdGraphNode* FM2SoundGraphAddNodeAction_NewAudioInsert::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	UEdGraphPin* FromPinConnection = nullptr;
	TArray<UM2SoundEdGraphNode*> Pins;
	if (FromPin)
	{
		UM2SoundEdGraphNode* AsM2SoundNode = Cast<UM2SoundEdGraphNode>(FromPin->GetOwningNode());
		if(AsM2SoundNode)
		{
			Pins = AsM2SoundNode->CurrentTrackOutputs;
		}
	}

	if(Pins.Num() > 0)
	{
		//FromPinConnection = Pins[0];
	}

	FGraphNodeCreator<UM2SoundAudioInsertNode> NodeCreator(*ParentGraph);
	UM2SoundAudioInsertNode* Node = NodeCreator.CreateUserInvokedNode();
	Node->Name = FName("Audio Insert");

	Node->Vertex = NewObject<UM2SoundPatch>(Node->GetSequencerData(), NAME_None, RF_Transactional);
	//Node->GetSequencerData()->AddVertex(Node->Vertex);

	if (FromPinConnection)
	{
		UE_LOG(LogTemp, Warning, TEXT("FromPinConnection: %s"), *FromPinConnection->GetOwningNode()->GetName());
		// if this is true we should recreate the connection between the new insert's output and the next node's input
		for(auto Pin : Node->Pins)
		{
			if(Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == "Track")
			{
				//ParentGraph->GetSchema()->TryCreateConnection(Pin, FromPinConnection);
			}
		}
	}
	//auto ParentGraph = Cast<UM2SoundGraph>(ParentGraph);
	
	Node->Vertex->OnVertexUpdated.AddDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
	Node->GetSequencerData()->AddVertex(Node->Vertex);

	NodeCreator.Finalize();

	ParentGraph->NotifyGraphChanged();

	return Node;
}
