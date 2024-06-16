// Fill out your copyright notice in the Description page of Project Settings.

#include "M2SoundEdGraphNodeBaseTypes.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"

#include "AudioParameter.h"
#include "IAudioParameterInterfaceRegistry.h"
#include "SGraphNode.h"
#include "Sound/SoundBase.h"
#include "EditorSlateWidgets/SM2SoundEdGraphNode.h"
#include "EditorSlateWidgets/SM2AudioOutputNode.h"
#include "EditorSlateWidgets/SM2MidiTrackGraphNode.h"



void UM2SoundGraphMidiOutputNode::AllocateDefaultPins()
{
	// Create any pin for testing
	CreatePin(EGPD_Input, "Track-Midi", FName("Track (Midi)", 0));
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
	// create midi track output pin
	CreatePin(EGPD_Output, "Track-Midi", FName("Track (Midi)", 0));

	//CreatePin(EGPD_Output, "Track", FName("Track", 0));
	Pins.Last()->DefaultValue = "Default";
}

TSharedPtr<SGraphNode> UM2SoundGraphInputNode::CreateVisualWidget()
{
	return SNew(SM2MidiTrackGraphNode, this);
}

inline TSharedPtr<SGraphNode> UM2SoundPatchContainerNode::CreateVisualWidget()
{
	return SNew(SM2SoundPatchContainerGraphNode<unDAW::Metasounds::FunDAWInstrumentRendererInterface>, this);
}

void UM2SoundPatchContainerNode::AllocateDefaultPins()
{
	// Create any pin for testing
	CreatePin(EGPD_Input, "Track-Midi", FName("Track (Midi)", 0));
	Pins.Last()->DefaultValue = "Default";

	CreatePin(EGPD_Output, "Track-Audio", FName("Track (Audio)", 0));
	Pins.Last()->DefaultValue = "Default";
}

void UM2SoundEdGraphNode::NodeConnectionListChanged()
{

	UEdGraphNode::NodeConnectionListChanged();

	//if no vertex probably recreating graph idk.
	if (!Vertex) return;

	return;


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
	//UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: PinConnectionListChanged, %s"), *Pin->GetName());
	//just print the pin data for now please
	if (!Pin) return;
	if (!Vertex) return;

	auto PinName = Pin->GetName();
	auto PinLinkedTo = Pin->LinkedTo.Num();
	auto PinDirection = Pin->Direction == EGPD_Input ? FString(TEXT("Input")) : FString(TEXT("Output"));
	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schema: PinConnectionListChanged, %s, %s, %d"), *PinName, *PinDirection, PinLinkedTo);

	//if we're either of the track input pins, we need to check if we have a connection
	if (Pin->Direction == EGPD_Input && (Pin->PinType.PinCategory == "Track-Midi" || Pin->PinType.PinCategory == "Track-Audio"))
	{
		if (PinLinkedTo == 0)
		{
			Vertex->BreakTrackInputConnection();
			AssignedTrackId = INDEX_NONE;
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
			auto LinkedToNode = LinkedToPin->GetOwningNode();
			auto AsM2Node = Cast<UM2SoundEdGraphNode>(LinkedToNode);
			auto LinkedToVertex = AsM2Node->Vertex;
			Vertex->MakeTrackInputConnection(LinkedToVertex);
			AssignedTrackId = AsM2Node->AssignedTrackId;
		}
		UpdateDownstreamTrackAssignment(AssignedTrackId);
		GetGraph()->NotifyGraphChanged();
	}

	// it's probably better if we just break inputs, and let the vertex handle the rest
	// still not sure how this would work when we have an external (in game) editor... but we'll get there.
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
	UE_LOG(LogTemp, Warning, TEXT("m2sound graph schemca: VertexUpdated, %s has %d outputs"), *Vertex->GetName(), Vertex->Outputs.Num());

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

		CreatePin(EGPD_Output, Pin.DataType, PinName);
		AutoGeneratedPins.Add(Pins.Last());

	}

	for (auto& [PinName, Pin] : Vertex->InPinsNew)
	{
		if (Pin.PinFlags & static_cast<uint8>(EM2SoundPinFlags::IsAutoManaged)) continue;

		CreatePin(EGPD_Input, Pin.DataType, PinName);
		AutoGeneratedPins.Add(Pins.Last());
	}

	OnNodeUpdated.ExecuteIfBound();
	GetGraph()->NotifyGraphChanged();
}

void UM2SoundGraphAudioOutputNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "Track-Audio", FName("Track (Audio)", 0));
	Pins.Last()->DefaultValue = "Default";
}

TSharedPtr<SGraphNode> UM2SoundGraphAudioOutputNode::CreateVisualWidget()
{
	return SNew(SM2AudioOutputNode, this);
}

TSharedPtr<SGraphNode> UM2SoundAudioInsertNode::CreateVisualWidget()
{
	return SNew(SM2SoundPatchContainerGraphNode<unDAW::Metasounds::FunDAWCustomInsertInterface>, this);
}

void UM2SoundAudioInsertNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "Track-Audio", FName("Track (Audio)", 0));
	Pins.Last()->DefaultValue = "Default";

	CreatePin(EGPD_Output, "Track-Audio", FName("Track (Audio)", 0));
	Pins.Last()->DefaultValue = "Default";
}
