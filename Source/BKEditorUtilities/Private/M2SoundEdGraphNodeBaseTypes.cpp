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
#include "ToolMenu.h"
//#include "Framework/Commands/UICommandList.h"
#include "EditorSlateWidgets/SM2MidiTrackGraphNode.h"





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

	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: NodeConnectionListChanged, %s"), *GetName());

	//if no vertex probably recreating graph idk.
	if (!Vertex) return;

	return;


	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: NodeConnectionListChanged, %s"), *Vertex->GetName());

	//update CurrentTrackOuputs
	CurrentTrackOutputs.Empty();
	TArray<UM2SoundEdGraphNode*> Outputs;
	UM2SoundEdGraphNode* FoundTrackInput = nullptr;

	//GetInputs

	for (auto Pin : Pins)
	{
		if (Pin->Direction == EGPD_Output && (Pin->PinType.PinCategory == "Track-Midi" || Pin->PinType.PinCategory == "Track-Audio"))
		{
			//if Pin has connections, cast them to M2SoundEdGraphNode and add them to CurrentTrackOutputs
			if (Pin->LinkedTo.Num() > 0)
			{
				for (auto LinkedPin : Pin->LinkedTo)
				{
					if (LinkedPin->GetOwningNode()->IsA<UM2SoundEdGraphNode>())
					{
						Outputs.Add(Cast<UM2SoundEdGraphNode>(LinkedPin->GetOwningNode()));
					}
				}
			}
		}

		//find if track input is connected
		if (Pin->Direction == EGPD_Input && (Pin->PinType.PinCategory == "Track-Midi" || Pin->PinType.PinCategory == "Track-Audio"))
		{
			if (Pin->LinkedTo.Num() > 0)
			{
				for (auto LinkedPin : Pin->LinkedTo)
				{
					if (LinkedPin->GetOwningNode()->IsA<UM2SoundEdGraphNode>())
					{
						FoundTrackInput = Cast<UM2SoundEdGraphNode>(LinkedPin->GetOwningNode());
					}
				}
			}
		}
	}

	//if meaningful connections (only track input is relevant here?) changed, inform the vertex so that the builder can be triggered

	for (auto Output : Outputs)
	{
		CurrentTrackOutputs.AddUnique(Output);
	}

	//check if we have a track input
	//if(FoundTrackInput)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Found Track Input, my vertex is %s"), *Vertex->GetName());
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("No Track Input Found, my vertex is %s"), *Vertex->GetName());
	//	Vertex->BreakTrackInputConnection();
	//}

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
			//Vertex->BreakTrackInputConnection();
			//AssignedTrackId = INDEX_NONE;
		}
		if (PinLinkedTo > 0)
		{
			//if we have a connection, we need to find the node that we're connected to
			//and then we need to find the vertex that the node is connected to
			//and then we need to connect our vertex to that vertex
			//this is a bit of a mess, but it's the only way to do it right now
			//we could probably do this in the graph, but it's probably better if we just break inputs, and let the vertex handle the rest
			// still not sure how this would work when we have an external (in game) editor... but we'll get there.
			auto LinkedToPin = Pin->LinkedTo[0];
			bool success = MakeConnection(Pin, LinkedToPin);


			//auto LinkedToNode = LinkedToPin->GetOwningNode();
			//while(UM2SoundRerouteNode* AsReroute = Cast<UM2SoundRerouteNode>(LinkedToNode))
			//{
			//	LinkedToNode = AsReroute->GetInputPin()->LinkedTo[0]->GetOwningNode();
			//}
			//UM2SoundEdGraphNode* AsM2Node =  Cast<UM2SoundEdGraphNode>(LinkedToNode);
			//auto LinkedToVertex = AsM2Node->Vertex;

			////now that we have the two vertexes, find their M2Pins and connect them
			////LinkedToVertex->OutputM2SoundPins.FindRef(LinkedToPin->GetFName())
			//UM2MetasoundLiteralPin* InLiteralPin = Cast<UM2MetasoundLiteralPin>(Vertex->InputM2SoundPins.FindRef(Pin->GetFName()).Get());
			//UM2MetasoundLiteralPin* OutLiteralPin = Cast<UM2MetasoundLiteralPin>(LinkedToVertex->OutputM2SoundPins.FindRef(LinkedToPin->GetFName()).Get());
			//bool success = GetSequencerData()->ConnectPins<UM2MetasoundLiteralPin>(InLiteralPin, OutLiteralPin);
			if(!success)
			{
				UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: PinConnectionListChanged, %s, %s, %d"), *PinName, *PinDirection, PinLinkedTo);
				Vertex->BuilderConnectionResults.Add(Pin->GetFName(), EMetaSoundBuilderResult::Failed);
			}
			else {
				//clear result
				if(Vertex->BuilderConnectionResults.Contains(Pin->GetFName()))	Vertex->BuilderConnectionResults.Remove(Pin->GetFName());
			}
			//Pin->


			//Vertex->MakeTrackInputConnection(LinkedToVertex);
			//AssignedTrackId = AsM2Node->AssignedTrackId;
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
	UM2SoundEdGraphNode* AsM2Node = Cast<UM2SoundEdGraphNode>(LinkedToNode);
	while (UM2SoundRerouteNode* AsReroute = Cast<UM2SoundRerouteNode>(LinkedToNode))
	{
		LinkedToNode = AsReroute->GetInputPin()->LinkedTo[0]->GetOwningNode();
	}
	//UM2SoundEdGraphNode* AsM2Node = Cast<UM2SoundEdGraphNode>(LinkedToNode);


	//if we're a composite pin, connect internal left rights
	if (A->PinType.PinCategory == "Track-Audio")
	{
		//if we don't have subpins we can find the metasound literals and connect them
		FName AudioTrackName = M2Sound::Pins::Categories::AudioTrack;
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

	AllocateDefaultPins();

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

	//clear autogen pins -- WE PROBABLY DON'T WANT TO DO THIS
	for (auto& Pin : AutoGeneratedPins)
	{
		Pin->BreakAllPinLinks();
		RemovePin(Pin);
	}

	AutoGeneratedPins.Empty();

	

	if (!bShowAdvanced)
	{
		GetGraph()->NotifyGraphChanged();
		return;
	}
	//create pins for meta sound ios

	//print num out pins new and in pins new
	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: Vertex has %d outputs and %d inputs"), Vertex->OutPinsNew.Num(), Vertex->InPinsNew.Num());
	for (auto& [PinName, Pin] : Vertex->OutPinsNew)
	{
		if (Pin.PinFlags & static_cast<uint8>(EM2SoundPinFlags::IsAutoManaged)) continue;

		CreatePin(EGPD_Output, FName(TEXT("MetasoundLiteral")),Pin.DataType, PinName);
		Pins.Last()->PinToolTip = Pin.DataType.ToString();
		AutoGeneratedPins.Add(Pins.Last());

	}

	for (auto& [PinName, Pin] : Vertex->InPinsNew)
	{
		if (Pin.PinFlags & static_cast<uint8>(EM2SoundPinFlags::IsAutoManaged)) continue;

		CreatePin(EGPD_Input, FName(TEXT("MetasoundLiteral")), Pin.DataType, PinName);
		AutoGeneratedPins.Add(Pins.Last());
	}

	OnNodeUpdated.ExecuteIfBound();
	GetGraph()->NotifyGraphChanged();
}

inline void UM2SoundEdGraphNode::AllocateDefaultPins() {

	//okay so in theory, we should check if the pins are stale in order to preserve connections, for now let's just empty pins.

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

			auto* NewComposite = CreatePin(EGPD_Input, FName(TEXT("Track-Audio")), FName("Track (Audio)"));
			FString ToolTip;
			GetGraph()->GetSchema()->ConstructBasicPinTooltip(*NewComposite, INVTEXT("Composite Audio Pin"), ToolTip);
			NewComposite->PinToolTip = ToolTip;
			CurrIndexOfAudioTrack = Pins.Num() - 1;


			//auto* NewLeft = CreatePin(EGPD_Input, FName(TEXT("MetasoundLiteral")), AsAudioTrackPin->AudioStreamL, PinName);
			//auto* NewRight = CreatePin(EGPD_Input, FName(TEXT("MetasoundLiteral")), AsAudioTrackPin->AudioStreamR, PinName);

			//NewLeft->ParentPin = NewComposite;
			//NewRight->ParentPin = NewComposite;


			

			//NewComposite->SubPins.Add(NewLeft);
			//NewComposite->SubPins.Add(NewRight);

			//NewLeft->SafeSetHidden(true);
			//NewRight->SafeSetHidden(true);

			//NewComposite->

			//Pins.Last()->DefaultValue = "Default
			continue;
		}

		if (Pin->IsA<UM2MetasoundLiteralPin>())
		{
			CreatePin(EGPD_Input, FName(TEXT("MetasoundLiteral")), Cast<UM2MetasoundLiteralPin>(Pin)->DataType, PinName);
			Pins.Last()->PinToolTip = Cast<UM2MetasoundLiteralPin>(Pin)->DataType.ToString();
			//Pins.Last()->DefaultValue = "Default";

			continue;
		}


	}
	if (CurrIndexOfAudioTrack != INDEX_NONE && CurrIndexOfAudioTrack != 0)	Swap(Pins.Last(), Pins[CurrIndexOfAudioTrack]);

	CurrIndexOfAudioTrack = INDEX_NONE;
	
	//Vertex->InputM2SoundPins.KeySort([](const FName& A, const FName& B) {return A == M2Sound::Pins::Categories::AudioTrack; });

	for (auto& [PinName, Pin] : Vertex->OutputM2SoundPins)
	{
		if (Pin->IsA<UM2AudioTrackPin>())
		{
			UM2AudioTrackPin* AsAudioTrackPin = Cast<UM2AudioTrackPin>(Pin);
			auto* NewComposite = CreatePin(EGPD_Output, FName(TEXT("Track-Audio")), FName("Track (Audio)"));
			Pins.Last()->PinToolTip = TEXT("Stereo Audio Channels");
			CurrIndexOfAudioTrack = Pins.Num() - 1;

			//auto* NewLeft = CreatePin(EGPD_Output, FName(TEXT("MetasoundLiteral")), AsAudioTrackPin->AudioStreamL, PinName);
			//auto* NewRight = CreatePin(EGPD_Output, FName(TEXT("MetasoundLiteral")), AsAudioTrackPin->AudioStreamR, PinName);


			continue;
		}

		if (Pin->IsA<UM2MetasoundLiteralPin>())
		{
			CreatePin(EGPD_Output, FName(TEXT("MetasoundLiteral")), Cast<UM2MetasoundLiteralPin>(Pin)->DataType, PinName);
			Pins.Last()->PinToolTip = Cast<UM2MetasoundLiteralPin>(Pin)->DataType.ToString();
			//Pins.Last()->DefaultValue = "Default";
			continue;
		}

	};

	if (CurrIndexOfAudioTrack != INDEX_NONE && CurrIndexOfAudioTrack != 0)	Swap(Pins.Last(), Pins[CurrIndexOfAudioTrack]);
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
