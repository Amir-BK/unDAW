// Fill out your copyright notice in the Description page of Project Settings.

#include "M2SoundEdGraphSchema.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"

#include "AudioParameter.h"
#include "IAudioParameterInterfaceRegistry.h"
#include "SGraphNode.h"
#include "Sound/SoundBase.h"
#include "M2SoundEdGraphNodeBaseTypes.h"
#include "EditorSlateWidgets/SM2SoundEdGraphNode.h"
#include "EditorSlateWidgets/SM2AudioOutputNode.h"
#include "UnDAWPreviewHelperSubsystem.h"
#include "Vertexes/M2VariMixerVertex.h"
#include "EditorSlateWidgets/SM2MidiTrackGraphNode.h"

void UM2SoundEdGraphSchema::SplitPin(UEdGraphPin* Pin, bool bNotify) const
{
	if (Pin->GetOwningNode()->CanSplitPin(Pin))
	{
		Pin->SafeSetHidden(true);
		//Pin->bOrphanedPin = true;

		for (auto& childPin : Pin->SubPins)
		{
			childPin->SafeSetHidden(false);
			//childPin->bOrphanedPin = false;
		}
	}
}

const FPinConnectionResponse UM2SoundEdGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	
	//get underlying vertexes and check for cycle

	const auto& AParentVertex = Cast<UM2SoundEdGraphNode>(A->GetOwningNode())->Vertex;
	const auto& BParentVertex = Cast<UM2SoundEdGraphNode>(B->GetOwningNode())->Vertex;




	
	if (A->Direction == B->Direction)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot connect output to output or input to input."));

	FName ACategory = A->PinType.PinCategory;
	FName BCategory = B->PinType.PinCategory;

	if(ACategory == BCategory)
	{
		bool bWillCauseLoop = AParentVertex->GetSequencerData()->WillConnectionCauseLoop(AParentVertex, BParentVertex);

		if (ACategory == "wildcard")
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, TEXT("Connect To Reroute"));
		}

		if (ACategory == "MetasoundLiteral")
		{
			// kinda crazy dunno if I got it right, if categories match OR only one is a wildcard then we can connect
			if (A->PinType.PinSubCategory == B->PinType.PinSubCategory || (!(B->PinType.PinSubCategory != "WildCard") != !(A->PinType.PinSubCategory != "WildCard")))
			{
				if(!bWillCauseLoop) 	return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, TEXT("Connect Metasound Literals, Type: ") + B->PinType.PinSubCategory.ToString());
			}
			else
			{
				return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot connect Metasound Literals of different types"));
			}
		}

		if (ACategory == "Track-Audio" && !bWillCauseLoop)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, TEXT("Connect Track Bus"));
		}



		if (bWillCauseLoop)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Connection will cause a loop"));
		}
	}
	else {
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not implemented by this schema."));
	}

	//if wild cards, get real categories of connected pins




	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not implemented by this schema."));
}

void UM2SoundEdGraphSchema::CreateMidiPinContextActions(FGraphContextMenuBuilder& ContextMenuBuilder, const FMemberInput& MidiInput) const
{
	FText CatergoryName = FText::FromString(MidiInput.DataType.ToString() + (" Inputs"));
	FText ToolTip = FText::FromString("Input Type: " + MidiInput.DataType.ToString());

	auto NewAction = MakeShared<FM2SoundGraphAddNodeAction_NewGraphInputNode>();
	NewAction->MetadataIndex = MidiInput.MetadataIndex;
	NewAction->UpdateSearchData(FText::FromName(MidiInput.Name), ToolTip, CatergoryName, FText::FromName(MidiInput.DataType));

	ContextMenuBuilder.AddAction(NewAction);
}

void UM2SoundEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	UEdGraphSchema::GetGraphContextActions(ContextMenuBuilder);

	auto* AsM2SoundGraph = Cast<UM2SoundGraph>(ContextMenuBuilder.CurrentGraph);
	const auto& SequencerData = AsM2SoundGraph->GetSequencerData();


	if (auto& FromPin = ContextMenuBuilder.FromPin)
	{
		if (FromPin->SubPins.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: FromPin has subpins"));
			auto TestAction = MakeShared<FM2SoundGraphAddNodeAction_NewAudioInsert>();
			TestAction->UpdateSearchData(FText::FromString("Test"), INVTEXT("Test"), INVTEXT("Test"), INVTEXT("Test"));
			ContextMenuBuilder.AddAction(TestAction);
		}

		if (FromPin->PinType.PinSubCategory == "MidiStream")
		{
			ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewInstrument>());
		}

		if (FromPin->PinType.PinCategory == "MetasoundLiteral")
		{
			//auto PromoteToGraphInputAction = MakeShared<FM2SoundGraphAddNodeAction_NewGraphInputNode>();

			switch (FromPin->Direction)
			{
			case EGPD_Input:
				//PromoteToGraphInputAction->UpdateSearchData(INVTEXT("Promote To Graph Input"), FText::FromString(""), FText::FromString(""), FText::FromName(""));
				
				ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphPromoteToGraphInputAction>());

				break;

			case EGPD_Output:
				ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphPromoteToGraphOutputAction>());
				break;

			case EGPD_MAX:
				break;

			default:
				break;

			}
			
			
			for (const auto& [InputName, Input] : SequencerData->CoreNodes.MemberInputMap)
			{

	
				
				if (FromPin->PinType.PinSubCategory != Input.DataType)
				{
					continue;
				}

				//if(Input.DataType == "MidiStream")
				//{
				//	CreateMidiPinContextActions(ContextMenuBuilder, Input);
				//	//ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewGraphInputNode>());
				//}
				//else {
				FText CatergoryName = FText::FromString(Input.DataType.ToString() + (" Inputs"));
				FText ToolTip = FText::FromString("Input Type: " + Input.DataType.ToString());

				auto NewAction = MakeShared<FM2SoundGraphAddNodeAction_NewGraphInputNode>();
				UE_LOG(LogTemp, Warning, TEXT("Metadata Index: %d"), Input.MetadataIndex);
				NewAction->MetadataIndex = Input.MetadataIndex;
				NewAction->UpdateSearchData(FText::FromName(InputName), ToolTip, CatergoryName, FText::FromName(Input.DataType));

				ContextMenuBuilder.AddAction(NewAction);

				//}
			}
		}

		if (FromPin->PinType.PinCategory == "Track-Audio")
		{
			//ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewAudioOutput>());
			ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewAudioInsert>());

			if (FromPin->Direction == EGPD_Output)
			{
				ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewVariMixerNode>());
			}
		}
	}

	if (!ContextMenuBuilder.FromPin)
	{
		//ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewOutput>());
		ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewInstrument>());
		//ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewAudioOutput>());
		ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewAudioInsert>());
		ContextMenuBuilder.AddAction(MakeShared<FM2SoundGraphAddNodeAction_NewVariMixerNode>());

		for (const auto& [InputName, Input] : SequencerData->CoreNodes.MemberInputMap)
		{
			FText CatergoryName = FText::FromString(Input.DataType.ToString() + (" Inputs"));
			FText ToolTip = FText::FromString("Input Type: " + Input.DataType.ToString());

			auto NewAction = MakeShared<FM2SoundGraphAddNodeAction_NewGraphInputNode>();
			NewAction->MetadataIndex = Input.MetadataIndex;
			NewAction->UpdateSearchData(FText::FromName(InputName), ToolTip, CatergoryName, FText::FromName(Input.DataType));

			ContextMenuBuilder.AddAction(NewAction);
		}
	}
}

inline bool UM2SoundEdGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
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
}


//set pin type colors

inline FLinearColor UM2SoundEdGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	//if wild card and connected to something get the color of that something
	//if(PinType.PinCategory == "WildCard")
	//{
	//		return GetPinTypeColor(PinType.PinSubCategory);
	//}

	UUNDAWSettings* Settings = UUNDAWSettings::Get();

	//Tracks are blue
	if (PinType.PinCategory == "Track-Audio")
	{
		return Settings->AudioPinTypeColor;
	}

	//metasound literals get the value from the metasound literal schema according to their data type
	if (PinType.PinCategory == "MetasoundLiteral")
	{
		if (Settings->CustomPinTypeColors.Contains(PinType.PinSubCategory))
		{
			return Settings->CustomPinTypeColors[PinType.PinSubCategory];
		}

		if (PinType.PinSubCategory == M2Sound::Pins::PinCategories::PinSubCategoryFloat)
		{
			return Settings->FloatPinTypeColor;
		}
		else if (PinType.PinSubCategory == M2Sound::Pins::PinCategories::PinSubCategoryInt32)
		{
			return Settings->IntPinTypeColor;
		}
		else if (PinType.PinSubCategory == M2Sound::Pins::PinCategories::PinSubCategoryBoolean)
		{
			return Settings->BooleanPinTypeColor;
		}
		else if (PinType.PinSubCategory == M2Sound::Pins::PinCategories::PinSubCategoryString)
		{
			return Settings->StringPinTypeColor;
		}
		else if (PinType.PinSubCategory == M2Sound::Pins::PinCategories::PinSubCategoryObject)
		{
			return Settings->ObjectPinTypeColor;
		}
		else
		{
			return Settings->DefaultPinTypeColor;
		}
	}

	return UEdGraphSchema::GetPinTypeColor(PinType);
}

void UM2SoundEdGraphSchema::OnPinConnectionDoubleCicked(UEdGraphPin* PinA, UEdGraphPin* PinB, const FVector2D& GraphPosition) const
{
	const FScopedTransaction Transaction(INVTEXT("M2Sound: Create Reroute Node"));

	const FVector2D NodeSpacerSize(42.0f, 24.0f);
	const FVector2D KnotTopLeft = GraphPosition - (NodeSpacerSize * 0.5f);

	UEdGraph* ParentGraph = PinA->GetOwningNode()->GetGraph();

	FGraphNodeCreator<UM2SoundRerouteNode> NodeCreator(*ParentGraph);
	UM2SoundRerouteNode* Node = NodeCreator.CreateUserInvokedNode();
	Node->NodePosX = KnotTopLeft.X;
	Node->NodePosY = KnotTopLeft.Y;
	//Node->Name = FName("Output");

	//Node->Vertex = NewObject<UM2SoundAudioOutput>(Node->GetSequencerData(),NAME_None, RF_Transactional);
	//Node->Vertex->OnVertexUpdated.AddDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
	//Node->GetSequencerData()->AddVertex(Node->Vertex);
	NodeCreator.Finalize();
	Node->GetInputPin()->PinType = PinA->PinType;
	Node->GetOutputPin()->PinType = PinB->PinType;

	PinA->BreakLinkTo(PinB);
	PinA->MakeLinkTo((PinA->Direction == EGPD_Output) ? Node->GetInputPin() : Node->GetOutputPin());
	PinB->MakeLinkTo((PinB->Direction == EGPD_Output) ? Node->GetInputPin() : Node->GetOutputPin());

	//UM2SoundRerouteNode* RerouteNode = FM2SoundGraphAddNodeAction::MakeNode<UM2SoundRerouteNode>(ParentGraph, nullptr);
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
		//NewNode->NodeConnectionListChanged();
		//FromPin->GetOwningNode()->NodeConnectionListChanged();
		break;
	}

	//NewNode->bHasCompilerMessage = true;
	//NewNode->GetDeprecationResponse()->Message = FText::FromString("This node is deprecated and will be removed in a future version of the plugin.");
	//cast to m2sound node and bind the vertex to the node
	UM2SoundEdGraphNode* Node = Cast<UM2SoundEdGraphNode>(NewNode);
	//if(IsValid(Node->Vertex))
	//{
	//	Node->Vertex->OnVertexUpdated.AddDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
	//}
	Node->Vertex->OnVertexUpdated.AddUniqueDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
	Node->GetGraph()->MapVertexToNode(Node->Vertex, Node);
	NewNode->GetGraph()->NotifyGraphChanged();

	return NewNode;
}

FM2SoundGraphToOutputAction::FM2SoundGraphToOutputAction(const TArray<UEdGraphPin*>& InSourcePins) : FEdGraphSchemaAction(INVTEXT("Meta"), INVTEXT("To Output"), INVTEXT("Creates a M2Sound output for each selected output pin."), 0)
, SourcePins(InSourcePins)
{}

void UM2SoundGraph::SaveVertexRangesToCache()
{
	//we'll need only the patches

	for (const auto& Vertex : SelectedVertices)
	{
		auto asPatchVertex = Cast<UM2SoundPatch>(Vertex);

		if (asPatchVertex)
		{
			//get the patch vertex and get the default ranges
			//asPatchVertex->GetDefaultRanges();
			asPatchVertex->SaveDefaultsToVertexCache();
		}
	}
}

TArray<UEdGraphPin*> UM2SoundGraph::GetSelectedPins(EEdGraphPinDirection Direction) const
{
	TArray<UEdGraphPin*> Pins;

	return MoveTemp(Pins);
}

//The initialize graph is not really safe to be called other than through the initialization sequence of the m2sound asset
void UM2SoundGraph::InitializeGraph()
{
	Nodes.Empty();
	VertexToNodeMap.Empty();

	CommandList = MakeShared<FUICommandList>();

	for (const auto& Vertex : GetSequencerData()->GetVertexes())
	{
		//create node in accordance to the vertex class, a little ugly but we have only three cases
		//for now check for the node class, we're looking for Patch Vertex, Audio Output Vertex and 'Input Handle Node' vertex

		UM2SoundEdGraphNode* Node = nullptr;

		if (Vertex->IsA<UM2SoundPatch>())
		{
			Node = CreateDefaultNodeForVertex<UM2SoundPatchContainerNode>(Vertex, FPlacementDefaults::InstrumentColumns);
		}

		if (Vertex->IsA<UM2SoundAudioOutput>())
		{
			Node =CreateDefaultNodeForVertex<UM2SoundGraphAudioOutputNode>(Vertex, FPlacementDefaults::OutputsColumnPosition);
		}

		if (Vertex->IsA<UM2SoundBuilderInputHandleVertex>())
		{
			auto InNode = CreateDefaultNodeForVertex< UM2SoundGraphInputNode>(Vertex, FPlacementDefaults::InputsColumnPosition);

			InNode->Name = FName(GetSequencerData()->GetTracksDisplayOptions(Vertex->TrackId).trackName);

			Node = InNode;
		}

		if (Vertex->IsA<UM2VariMixerVertex>())
		{
			Node = CreateDefaultNodeForVertex<UM2SoundVariMixerNode>(Vertex, FPlacementDefaults::InstrumentColumns);
		}

		Node->Vertex->OnVertexUpdated.AddUniqueDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
		Node->GetGraph()->MapVertexToNode(Node->Vertex, Node);
	}

	//for (const auto& [Vertex, Node] : VertexToNodeMap)
	//{
	//	if(auto& InputVertex = Vertex->MainInput)
	//	{
	//		AutoConnectTrackPinsForNodes(*VertexToNodeMap[InputVertex], *Node);
	//
	//	}
	//}

	PerformVertexToNodeBinding();
	GetSequencerData()->OnVertexAdded.AddUniqueDynamic(this, &UM2SoundGraph::OnVertexAdded);
	NotifyGraphChanged();
}

void UM2SoundGraph::PerformVertexToNodeBinding()
{
	for (const auto NodeAsObject : Nodes)
	{
		auto Node = Cast<UM2SoundEdGraphNode>(NodeAsObject);
		if (!Node) continue;

		UM2SoundVertex* Vertex = Node->Vertex;
		if (!Vertex) continue;

		//Vertex->OnVertexUpdated.AddUniqueDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
		Node->VertexUpdated();
	}
}



UEdGraphNode* FM2SoundGraphAddNodeAction_NewInstrument::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	auto DefaultPatchTest = FSoftObjectPath(TEXT("'/unDAW/Patches/System/Fusion.Fusion'"));
	FGraphNodeCreator<UM2SoundPatchContainerNode> NodeCreator(*ParentGraph);
	UM2SoundPatchContainerNode* Node = NodeCreator.CreateUserInvokedNode();
	auto* NewVertex = NewObject<UM2SoundPatch>(Node->GetSequencerData(), NAME_None, RF_Transactional);
	NewVertex->Patch = CastChecked<UMetaSoundPatch>(DefaultPatchTest.TryLoad());
	Node->Name = FName("Instrument");
	Node->Vertex = NewVertex;

	//Node->Err

	//Node->Vertex->OnVertexUpdated.AddDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
	Node->GetSequencerData()->AddVertex(Node->Vertex);
	NodeCreator.Finalize();
	return Node;
}

UEdGraphNode* FM2SoundGraphAddNodeAction_NewAudioOutput::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundGraphAudioOutputNode> NodeCreator(*ParentGraph);
	UM2SoundGraphAudioOutputNode* Node = NodeCreator.CreateUserInvokedNode();
	Node->Name = FName("Audio Output");

	Node->Vertex = NewObject<UM2SoundAudioOutput>(Node->GetSequencerData(), NAME_None, RF_Transactional);
	//Node->Vertex->OnVertexUpdated.AddDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
	Node->GetSequencerData()->AddVertex(Node->Vertex);

	Node->Vertex->SequencerData = Node->GetSequencerData();
	NodeCreator.Finalize();

	return Node;
}

UEdGraphNode* FM2SoundGraphAddNodeAction_NewAudioInsert::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundAudioInsertNode> NodeCreator(*ParentGraph);
	UM2SoundAudioInsertNode* Node = NodeCreator.CreateUserInvokedNode();
	Node->Name = FName("Audio Insert");

	auto NewPatchVertex = FVertexCreator::CreateVertex<UM2SoundPatch>(Node->GetSequencerData());

	Node->Vertex = NewPatchVertex;

	auto DefaultPatchTest = FSoftObjectPath(TEXT("'/unDAW/Patches/System/unDAW_PassThroughInsert.unDAW_PassThroughInsert'"));
	NewPatchVertex->Patch = CastChecked<UMetaSoundPatch>(DefaultPatchTest.TryLoad());

	Node->GetSequencerData()->AddVertex(Node->Vertex);
	//Node->Vertex->OnVertexUpdated.AddDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
	NodeCreator.Finalize();

	//ParentGraph->NotifyGraphChanged();

	return Node;
}

UEdGraphNode* FM2SoundGraphAddNodeAction_NewGraphInputNode::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundGraphInputNode> NodeCreator(*ParentGraph);

	UM2SoundGraphInputNode* Node = NodeCreator.CreateUserInvokedNode();

	//acquire track metadata
	const auto& SequencerData = Cast<UM2SoundGraph>(ParentGraph)->GetSequencerData();
	const auto& TrackMetadata = SequencerData->M2TrackMetadata;
	const auto& SearchString = GetMenuDescription().ToString();

	//auto NewInputVertex = NewObject<UM2SoundMidiInputVertex>(Node->GetSequencerData(), NAME_None, RF_Transactional);

	auto InVertex = FVertexCreator::CreateVertex<UM2SoundBuilderInputHandleVertex>(Node->GetSequencerData());
	Node->Vertex = InVertex;
	Node->Vertex->TrackId = MetadataIndex;
	UE_LOG(LogTemp, Warning, TEXT("Track Id: %d"), MetadataIndex);
	InVertex->MemberName = FName(*SearchString);
	//InVertex->TrackPrefix = FString::Printf(TEXT("Tr%d_Ch%d."), TrackMetadata[MyMetadata].TrackIndexInParentMidi, TrackMetadata[MyMetadata].ChannelIndexInParentMidi);
	//Node->Vertex->OnVertexUpdated.AddDynamic(Node, &UM2SoundEdGraphNode::VertexUpdated);
	Node->GetSequencerData()->AddVertex(Node->Vertex);

	NodeCreator.Finalize();

	ParentGraph->NotifyGraphChanged();
	//find the track metadata that corresponds to the input

	return Node;
}

UEdGraphNode* FM2SoundGraphAddNodeAction_NewVariMixerNode::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundVariMixerNode> NodeCreator(*ParentGraph);

	auto Node = NodeCreator.CreateUserInvokedNode();
	Node->Vertex = FVertexCreator::CreateVertex<UM2VariMixerVertex>(Node->GetSequencerData());
	Node->Name = FName("VariMixer");
	Node->GetSequencerData()->AddVertex(Node->Vertex);

	NodeCreator.Finalize();

	return Node;
}

UEdGraphNode* FM2SoundGraphPromoteToGraphInputAction::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	FGraphNodeCreator<UM2SoundDynamicGraphInputNode> NodeCreator(*ParentGraph);
	auto Node = NodeCreator.CreateUserInvokedNode();

	Node->Vertex = FVertexCreator::CreateVertex<UM2SoundDynamicGraphInputVertex>(Node->GetSequencerData());
	Node->GetSequencerData()->AddVertex(Node->Vertex);
	//Node->Name = FName("Graph Input");

	NodeCreator.Finalize();
	
	return Node;
}

UEdGraphNode* FM2SoundGraphPromoteToGraphOutputAction::MakeNode(UEdGraph* ParentGraph, UEdGraphPin* FromPin)
{
	return nullptr;
}
