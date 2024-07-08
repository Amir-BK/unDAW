// Fill out your copyright notice in the Description page of Project Settings.

#include "M2SoundEdGraphNodeBaseTypes.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"

#include "AudioParameter.h"
#include "IAudioParameterInterfaceRegistry.h"
#include "SGraphNode.h"
#include "SGraphNodeKnot.h"
#include "Sound/SoundBase.h"
#include "Framework/Commands/GenericCommands.h"
#include "EditorSlateWidgets/SM2SoundEdGraphNode.h"
#include "EditorSlateWidgets/SM2AudioOutputNode.h"
#include "SequenceAssetEditor/DAWEditorCommands.h"
#include "ToolMenu.h"
//#include "Framework/Commands/UICommandList.h"
#include "EditorSlateWidgets/SM2MidiTrackGraphNode.h"






TSharedPtr<class SGraphNode> FM2SoundGraphPanelNodeFactory::CreateNode(UEdGraphNode* InNode) const
{
	return nullptr;
}


FText UM2SoundGraphConsumer::GetPinDisplayName(const UEdGraphPin* Pin) const
{
	if (Pin->Direction == EGPD_Output) return FText::FromName(Pin->PinName);
	int32 Index = Pins.Find(const_cast<UEdGraphPin*>(Pin));
	//check(Index != INDEX_NONE && Vertex);
	//if (Index >= Vertex->GetInputInfo().Num()) return INVTEXT("This pin should not exist! Remove and re-add the node.");
	//return Vertex->GetInputInfo()[Index].DisplayName;

	return INVTEXT("M2Sound Pin");
}


TSharedPtr<SGraphNode> UM2SoundGraphInputNode::CreateVisualWidget()
{
	return SNew(SM2MidiTrackGraphNode, this);
}

inline TSharedPtr<SGraphNode> UM2SoundPatchContainerNode::CreateVisualWidget()
{
	return SNew(SM2SoundPatchContainerGraphNode<unDAW::Metasounds::FunDAWInstrumentRendererInterface>, this);
}



void UM2SoundEdGraphNode::NodeConnectionListChanged()
{

	UEdGraphNode::NodeConnectionListChanged();

	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: NodeConnectionListChanged should do nothing, %s"), *GetName());


	GetGraph()->NotifyGraphChanged();


}

void UM2SoundEdGraphNode::PinConnectionListChanged(UEdGraphPin* Pin)
{
	//just print the pin data for now please
	if (!Pin) return;
	if (!Vertex) return;

	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: PinConnectionListChanged, %s"), *Pin->GetName());

	auto PinName = Pin->GetName();
	auto PinLinkedTo = Pin->LinkedTo.Num();
	auto PinDirection = Pin->Direction == EGPD_Input ? FString(TEXT("Input")) : FString(TEXT("Output"));
	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: PinConnectionListChanged, %s, %s, %d"), *PinName, *PinDirection, PinLinkedTo);

	//if we're either of the track input pins, we need to check if we have a connection
	if (Pin->Direction == EGPD_Input)
	{
		if (PinLinkedTo == 0)
		{
			//if pin had a connection and now doesn't, we need to break the connection
			UM2Pins* UnderlyingPin = Vertex->InputM2SoundPins.FindRef(Pin->GetFName());
				
			auto* CurrentConnection = UnderlyingPin->LinkedPin;
			if (CurrentConnection)
			{
				//break the connection
				//CurrentConnection->LinkedPin = nullptr;
			}

			if(auto* AsAudioTrackPin = Cast<UM2AudioTrackPin>(UnderlyingPin))
			{
				//if we're an audio track pin, we need to break the connection
				Vertex->GetSequencerData()->BreakPinConnection<UM2AudioTrackPin>(AsAudioTrackPin);
			}
			else
			{
				//cast to literal pin and break connection
				Vertex->GetSequencerData()->BreakPinConnection<UM2MetasoundLiteralPin>(Cast<UM2MetasoundLiteralPin>(UnderlyingPin));
			}

			


		}
		if (PinLinkedTo > 0)
		{

			//we need to check if we're currently connected to a vertex, if so, break that connection;
			auto* CurrentConnection = Vertex->InputM2SoundPins.FindRef(Pin->GetFName())->LinkedPin;
			if (CurrentConnection)
			{
				//break the connection
				//CurrentConnection->LinkedPin = nullptr;
			}
			

			auto LinkedToPin = Pin->LinkedTo[0];
			bool success = MakeConnection(Pin, LinkedToPin);


			if(!success)
			{
				UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: PinConnectionListChanged, %s, %s, %d"), *PinName, *PinDirection, PinLinkedTo);
				Vertex->BuilderConnectionResults.Add(Pin->GetFName(), EMetaSoundBuilderResult::Failed);
			}
			else {
				//clear result
				if(Vertex->BuilderConnectionResults.Contains(Pin->GetFName()))	Vertex->BuilderConnectionResults.Remove(Pin->GetFName());
			}

		}

		

		UpdateDownstreamTrackAssignment(AssignedTrackId);
		GetGraph()->NotifyGraphChanged();
	}



	// it's probably better if we just break inputs, and let the vertex handle the rest
	// still not sure how this would work when we have an external (in game) editor... but we'll get there.
}


bool UM2SoundEdGraphNode::MakeConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	
	//first resolve potential reroutes
	auto LinkedToNode = B->GetOwningNode();
	while (UM2SoundRerouteNode* AsReroute = Cast<UM2SoundRerouteNode>(LinkedToNode))
	{
		LinkedToNode = AsReroute->GetInputPin()->LinkedTo[0]->GetOwningNode();
	}

	auto ANode = A->GetOwningNode();
	//also need to resolve reroutes on the input side
	while (UM2SoundRerouteNode* AsReroute = Cast<UM2SoundRerouteNode>(ANode))
	{
		ANode = AsReroute->GetOutputPin()->LinkedTo[0]->GetOwningNode();
	}

	//UM2SoundEdGraphNode* AsM2Node = Cast<UM2SoundEdGraphNode>(ANode);
	UM2SoundEdGraphNode* AsM2Node = Cast<UM2SoundEdGraphNode>(LinkedToNode);


	//if we're a composite pin, connect internal left rights
	if (A->PinType.PinCategory == "Track-Audio")
	{
		//if we don't have subpins we can find the metasound literals and connect them
		FName AudioTrackName = M2Sound::Pins::AutoDiscovery::AudioTrack;
		UM2AudioTrackPin* InAudioTrackPin = Cast<UM2AudioTrackPin>(Vertex->InputM2SoundPins.FindRef(AudioTrackName).Get());
		UM2AudioTrackPin* OutAudioTrackPin = Cast<UM2AudioTrackPin>(AsM2Node->Vertex->OutputM2SoundPins.FindRef(AudioTrackName).Get());
		return GetSequencerData()->ConnectPins<UM2AudioTrackPin>(InAudioTrackPin, OutAudioTrackPin);
	}

	// if we don't have subpins we can find the metasound literals and connect them


	auto LinkedToVertex = AsM2Node->Vertex;

	UM2MetasoundLiteralPin* InLiteralPin = Cast<UM2MetasoundLiteralPin>(Vertex->InputM2SoundPins.FindRef(A->GetFName()).Get());
	UM2MetasoundLiteralPin* OutLiteralPin = Cast<UM2MetasoundLiteralPin>(LinkedToVertex->OutputM2SoundPins.FindRef(B->GetFName()).Get());
	return GetSequencerData()->ConnectPins<UM2MetasoundLiteralPin>(InLiteralPin, OutLiteralPin);


}

const void UM2SoundEdGraphNode::SplitPin(const UEdGraphPin* Pin) const
{
	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: SplitPin, %s"), *Pin->GetName());

}

void UM2SoundEdGraphNode::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	FM2SoundNodeCommands::Get().SetPinAsColorSource;
	FUICommandList CommandList;
	//GetGraph()->

	if (Context->Pin)
	{
		if (CanSplitPin(Context->Pin))
		{
			FToolMenuSection& Section = Menu->AddSection("M2SoundEdGraphNode", FText::FromString("M2SoundEdGraphNode"));
			FUIAction Action = FUIAction(FExecuteAction::CreateLambda([this, Context]() { SplitPin(Context->Pin); }));
			
			const FText Label = INVTEXT("Add Folder");
			const FText ToolTipText = INVTEXT("Add Folder to the current selected folder");
			//MenuBuilder.AddMenuEntry(Label, ToolTipText, FSlateIcon(), Action);
			Section.AddMenuEntry("SplitPin", FText::FromString("Split Pin"), FText::FromString("Split Pin"), FSlateIcon(), Action);
		}

		if(Context->Pin->Direction == EGPD_Input)
		{
			FToolMenuSection& Section = Menu->AddSection("M2SoundEdGraphNode", FText::FromString("M2SoundEdGraphNode"));
			//FM2SoundNodeCommands::Get().SetPinAsColorSource.Get()-B
			CommandList.MapAction(FM2SoundNodeCommands::Get().SetPinAsColorSource, FExecuteAction::CreateLambda([this, Context]() { SplitPin(Context->Pin); })
			, FCanExecuteAction(), FIsActionChecked::CreateLambda([this, Context]() { return true; }));
			//FM2SoundNodeCommands::Get().SetPinAsColorSource
			//MenuBuilder.AddMenuEntry(Label, ToolTipText, FSlateIcon(), Action);
			Section.AddMenuEntry(FM2SoundNodeCommands::Get().SetPinAsColorSource);
		}
	}
}

bool CheckForErrorMessagesAndReturnIfAny(const UM2SoundEdGraphNode* Node, FString& OutErrorMsg)
{
	//this would be the errors from the interfaces and the such
	if (Node->Vertex->bHasErrors)
	{
		OutErrorMsg = Node->Vertex->VertexErrors;
		return true;
	}

	//now we traverse the builder results and check for errors
	//EMetaSoundBuilderResult

	bool bHasErrors = false;

	for (const auto& [ErrName, Result] : Node->Vertex->BuilderResults)
	{
		if (Result == EMetaSoundBuilderResult::Failed)
		{
			OutErrorMsg += ErrName.ToString() + "- Builder Operation Failed\n";
			bHasErrors = true;
		}
	}

	return bHasErrors;
}

void UM2SoundEdGraphNode::VertexUpdated()
{
	//UE_LOG(LogTemp, Warning, TEXT("m2sound graph schemca: VertexUpdated, %s has %d outputs"), *Vertex->GetName(), Vertex->Outputs.Num());
	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: VertexUpdated, %s"), *GetName());
	AllocateDefaultPins();
	SyncVertexConnections();

	//assuming each vertex only implements one interface, we can keep most of the logic here
	Audio::FParameterInterfacePtr interface = nullptr;

	if (IsA<UM2SoundPatchContainerNode>())
	{
		interface = unDAW::Metasounds::FunDAWInstrumentRendererInterface::GetInterface();
	}

	if (IsA<UM2SoundPatchContainerNode>())
	{
		auto AsPatch = Cast<UM2SoundPatch>(Vertex);
		//if patch is valid, get its name and give it to this node
		if (AsPatch->Patch)
		{
			Name = FName(AsPatch->Patch->GetName());
		}
		else
		{
			Name = "No Patch Selected";
		}
	}

	bHasCompilerMessage = CheckForErrorMessagesAndReturnIfAny(this, ErrorMsg);


	OnNodeUpdated.ExecuteIfBound();
	GetGraph()->NotifyGraphChanged();
}

void UM2SoundEdGraphNode::SyncVertexConnections() const
{
	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: SyncVertexConnections, %s"), *GetName());
	
	for(auto Pin : Pins)
	{
		// if not input pin contiue;
		//if(Pin->Direction == EGPD_Output) continue;
		EEdGraphPinDirection PinReverseDirection = Pin->Direction == EGPD_Input ? EGPD_Output : EGPD_Input;
		//check if Pin underlying M2Pins has a connection, if so we need to find the node it's connected to and connect it
		UM2Pins* UnderlyingPin = Vertex->InputM2SoundPins.FindRef(Pin->GetFName()).Get();
		if(!UnderlyingPin) UnderlyingPin = Vertex->OutputM2SoundPins.FindRef(Pin->GetFName()).Get();
		if(!UnderlyingPin) continue;

		if(auto LinkedToM2Pin = UnderlyingPin->LinkedPin)
		{
			//find the node that the underlying pin is connected to
			//find the pin that the underlying pin is connected to
			//connect the pins
			auto* LinkedToNode = GetGraph()->GetNodeForVertex(UnderlyingPin->LinkedPin->ParentVertex);
			if (!LinkedToNode) {
				UE_LOG(LogTemp, Warning, TEXT("What and why"));
				continue;
			}

			auto* LinkedToPin = LinkedToNode->FindPin(UnderlyingPin->LinkedPin->Name, PinReverseDirection);
			if(!LinkedToPin) continue;

			Pin->LinkedTo.AddUnique(LinkedToPin);
			LinkedToPin->LinkedTo.AddUnique(Pin);
		}
		
		//if(Pin->Direction == EGPD_Input)
		//{
		//	if(Pin->LinkedTo.Num() > 0)
		//	{
		//		for(auto LinkedPin : Pin->LinkedTo)
		//		{
		//			if(LinkedPin->GetOwningNode()->IsA<UM2SoundEdGraphNode>())
		//			{
		//				//GetGraph()->GetSchema()->TryCreateConnection(Pin, LinkedPin);
		//			}
		//		}
		//	}
		//}
	}
}

inline void UM2SoundEdGraphNode::AllocateDefaultPins() {

	//okay so in theory, we should check if the pins are stale in order to preserve connections, for now let's just empty pins.

	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: AllocateDefaultPins, %s"), *GetName());

	TArray<UEdGraphPin*> PinsCopy = Pins;

	for (auto Pin : PinsCopy)
	{
		RemovePin(Pin);
	}

	int CurrIndexOfAudioTrack = INDEX_NONE;

	for (auto& [PinName, Pin] : Vertex->InputM2SoundPins)
	{
		if (Pin->IsA<UM2AudioTrackPin>())
		{
			
			UM2AudioTrackPin* AsAudioTrackPin = Cast<UM2AudioTrackPin>(Pin);

			auto* NewComposite = CreatePin(EGPD_Input, FName(TEXT("Track-Audio")), M2Sound::Pins::AutoDiscovery::AudioTrack);
			FString ToolTip;
			GetGraph()->GetSchema()->ConstructBasicPinTooltip(*NewComposite, INVTEXT("Composite Audio Pin"), ToolTip);
			NewComposite->PinToolTip = ToolTip;
			CurrIndexOfAudioTrack = Pins.Num() - 1;

			continue;
		}

		if (Pin->IsA<UM2MetasoundLiteralPin>())
		{
			CreatePin(EGPD_Input, FName(TEXT("MetasoundLiteral")), Cast<UM2MetasoundLiteralPin>(Pin)->DataType, PinName);
			Pins.Last()->PinToolTip = Cast<UM2MetasoundLiteralPin>(Pin)->DataType.ToString();


			continue;
		}


	}
	if (CurrIndexOfAudioTrack != INDEX_NONE && CurrIndexOfAudioTrack != 0)	Swap(Pins[0], Pins[CurrIndexOfAudioTrack]);

	CurrIndexOfAudioTrack = INDEX_NONE;
	
	//Vertex->InputM2SoundPins.KeySort([](const FName& A, const FName& B) {return A == M2Sound::Pins::Categories::AudioTrack; });

	for (auto& [PinName, Pin] : Vertex->OutputM2SoundPins)
	{
		if (Pin->IsA<UM2AudioTrackPin>())
		{
			UM2AudioTrackPin* AsAudioTrackPin = Cast<UM2AudioTrackPin>(Pin);
			auto* NewComposite = CreatePin(EGPD_Output, FName(TEXT("Track-Audio")), M2Sound::Pins::AutoDiscovery::AudioTrack);
			Pins.Last()->PinToolTip = TEXT("Stereo Audio Channels");
			CurrIndexOfAudioTrack = Pins.Num() - 1;

			continue;
		}

		if (Pin->IsA<UM2MetasoundLiteralPin>())
		{
			CreatePin(EGPD_Output, FName(TEXT("MetasoundLiteral")), Cast<UM2MetasoundLiteralPin>(Pin)->DataType, PinName);
			Pins.Last()->PinToolTip = Cast<UM2MetasoundLiteralPin>(Pin)->DataType.ToString();
		
			continue;
		}

	};

	if (CurrIndexOfAudioTrack != INDEX_NONE && CurrIndexOfAudioTrack != 0)	Swap(Pins[0], Pins[CurrIndexOfAudioTrack]);
}


TSharedPtr<SGraphNode> UM2SoundGraphAudioOutputNode::CreateVisualWidget()
{
	return SNew(SM2AudioOutputNode, this);
}

TSharedPtr<SGraphNode> UM2SoundAudioInsertNode::CreateVisualWidget()
{
	return SNew(SM2SoundPatchContainerGraphNode<unDAW::Metasounds::FunDAWCustomInsertInterface>, this);
}


TSharedPtr<SGraphNode> UM2SoundVariMixerNode::CreateVisualWidget()
{
	return SNew(SM2VariMixerNode, this);
}



void UM2SoundVariMixerNode::NodeConnectionListChanged()
{
	//if we don't have any free inputs, create another pin
	int NumFreeInputs = 0;
	for (auto Pin : Pins)
	{
		if (Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() == 0)
		{
			NumFreeInputs++;
		}
	}

	if (NumFreeInputs == 0)
	{
		CreatePin(EGPD_Input, "Track-Audio", FName("Track (Audio)", 0));
		Pins.Last()->DefaultValue = "Default";
	}
}

TSharedPtr<SGraphNode> UM2SoundRerouteNode::CreateVisualWidget()
{
	return SNew(SGraphNodeKnot, this);
}
