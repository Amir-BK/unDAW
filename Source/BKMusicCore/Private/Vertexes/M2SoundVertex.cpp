#include "Vertexes/M2SoundVertex.h"
#include "M2SoundGraphRenderer.h"
#include "M2SoundGraphStatics.h"
#include "Metasound.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"
#include "M2SoundGraphRenderer.h"

DEFINE_LOG_CATEGORY(unDAWVertexLogs);

void UM2SoundVertex::BreakTrackInputConnection()
{
	//for debugging, print vertex name and track id
	UE_LOG(unDAWDataLogs, Verbose, TEXT("BreakTrackInputConnection %s"), *GetName())
		MainInput->BreakTrackOutputConnection(this);
	MainInput = nullptr;
	TrackId = INDEX_NONE;

	//traverse outputs and update TrackId
	for (auto Output : Outputs)
	{
		Output->TrackId = INDEX_NONE;
	}

	UpdateConnections();

}

void UM2SoundVertex::MakeTrackInputConnection(UM2SoundVertex* InputVertex)
{
	//for debugging, print vertex name and track id
	UE_LOG(unDAWVertexLogs, Verbose, TEXT("MakeTrackInputConnection %s"), *GetName())

		//if we already have a main input and it's different from the new one break it
		if (MainInput && MainInput != InputVertex)
		{
			BreakTrackInputConnection();
		}

	//traverse outputs and update TrackId
	for (auto Output : Outputs)
	{
		Output->TrackId = InputVertex->TrackId;
	}

	//set the new input

	MainInput = InputVertex;
	TrackId = InputVertex->TrackId;
	//VertexNeedsBuilderUpdates();
	UpdateConnections();
	MainInput->RegisterOutputVertex(this);
}

void UM2SoundVertex::BreakTrackOutputConnection(UM2SoundVertex* OutputVertex)
{
	//for debugging, print vertex name and track id
	UE_LOG(unDAWVertexLogs, Verbose, TEXT("BreakTrackOutputConnection"))
	UnregisterOutputVertex(OutputVertex);
}

void UM2SoundVertex::RegisterOutputVertex(UM2SoundVertex* OutputVertex)
{
	Outputs.Add(OutputVertex);

}

bool UM2SoundVertex::UnregisterOutputVertex(UM2SoundVertex* OutputVertex)
{
	if(Outputs.Contains(OutputVertex))
	{
		Outputs.Remove(OutputVertex);
		return true;
	}
	
	return false;
}

UDAWSequencerData* UM2SoundVertex::GetSequencerData() const
{
	return SequencerData;
}

void UM2SoundVertex::VertexNeedsBuilderUpdates()
{
	UE_LOG(unDAWVertexLogs, Verbose, TEXT("Vertex needs builder updates!"))
	BuildVertex();
	CollectParamsForAutoConnect();
	UpdateConnections();

	//OnVertexNeedsBuilderConnectionUpdates.Broadcast(this);
}

void UM2SoundVertex::VertexConnectionsChanged()
{
	UE_LOG(unDAWVertexLogs, Verbose, TEXT("Vertex connections changed!"))
	UpdateConnections();
	//OnVertexNeedsBuilderConnectionUpdates.Broadcast(this);
}

void UM2SoundVertex::TransmitAudioParameter(FAudioParameter Parameter)
{
	if (GetSequencerData())
	{
		GetSequencerData()->ReceiveAudioParameter(Parameter);
	}
	else {
		UE_LOG(unDAWVertexLogs, Error, TEXT("Outer is not sequencer data FFS!"))
	}
}

inline bool ResultToBool(EMetaSoundBuilderResult& Result)
{
	return Result == EMetaSoundBuilderResult::Succeeded;
}

void UM2SoundVertex::CollectParamsForAutoConnect()
{
	//called right after the vertex is created, we should collect the parameters that are exposed by the vertex and can be auto connected
	//to other nodes in the metasound graph
	AutoConnectInPins.Empty();
	AutoConnectOutPins.Empty();

	EMetaSoundBuilderResult BuildResult;

	auto& BuilderSubsystems = SequencerData->MSBuilderSystem;
	auto& BuilderContext = SequencerData->BuilderContext;

	//find the inputs and outputs of the node
	//InPins = BuilderContext->FindNodeInputs(NodeHandle, BuildResult);
	for(const auto& Input : InPins)
	{
		FName NodeName;
		FName DataType;
		BuilderContext->GetNodeInputData(Input, NodeName, DataType, BuildResult);
		
		if(NodeName == FName(TEXT("unDAW Instrument.MidiStream")))
		{
			AutoConnectInPins.Add(EVertexAutoConnectionPinCategory::MidiTrackStream, Input);
			continue;
		}
		
		if(NodeName == FName(TEXT("unDAW Instrument.MidiTrack")))
		{
			AutoConnectInPins.Add(EVertexAutoConnectionPinCategory::MidiTrackTrackNum, Input);
			continue;
		}

		//also check the insert names unDAW Insert.Audio In L etc
		if(NodeName.ToString().Contains(TEXT("unDAW Insert.Audio In L")))
		{
			AutoConnectInPins.Add(EVertexAutoConnectionPinCategory::AudioStreamL, Input);
			continue;
		}

		if(NodeName.ToString().Contains(TEXT("unDAW Insert.Audio In R")))
		{
			AutoConnectInPins.Add(EVertexAutoConnectionPinCategory::AudioStreamR, Input);
			continue;
		}

	}

	for (const auto& Output : OutPins)
	{
		FName NodeName;
		FName DataType;
		BuilderContext->GetNodeOutputData(Output, NodeName, DataType, BuildResult);

		if (NodeName == FName(TEXT("unDAW Instrument.Audio L")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::AudioStreamL, Output);
			continue;
		}

		if (NodeName == FName(TEXT("unDAW Instrument.Audio R")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::AudioStreamR, Output);
			continue;
		}

		if (NodeName == FName(TEXT("MIDI Stream")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::MidiTrackStream, Output);
			continue;
		}

		if (NodeName == FName(TEXT("Track")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::MidiTrackTrackNum, Output);
			continue;
		}

		//insert audio outputs unDAW Insert.Audio L 

		if (NodeName == FName(TEXT("unDAW Insert.Audio L")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::AudioStreamL, Output);
			continue;
		}

		if (NodeName == FName(TEXT("unDAW Insert.Audio R")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::AudioStreamR, Output);
			continue;
		}

	}


}



void UM2SoundAudioOutput::BuildVertex()
{
	auto& BuilderSubsystems = SequencerData->MSBuilderSystem;
	auto& BuilderContext = SequencerData->BuilderContext;
	BuilderResults.Empty();

	EMetaSoundBuilderResult BuildResult;
	AudioOutput = SequencerData->CoreNodes.GetFreeMasterMixerAudioOutput(BuilderContext);
	BuilderResults.Add(FName(TEXT("Assigned Output")), EMetaSoundBuilderResult::Succeeded);
	GainParameterName = AudioOutput.OutputName;

	FName OutDataType;
	auto NewGainInput = BuilderContext->AddGraphInputNode(GainParameterName, TEXT("float"), BuilderSubsystems->CreateFloatMetaSoundLiteral(1.f, OutDataType), BuildResult);
	OutPins.Add(NewGainInput);
	BuilderContext->ConnectNodes(NewGainInput, AudioOutput.GainParameterInputHandle, BuildResult);
	BuilderResults.Add(FName(TEXT("Assigned Gain Param")), BuildResult);

}

void UM2SoundAudioOutput::UpdateConnections()
{
	//if no main input, do nothing
	BuilderConnectionResults.Empty();
	if (!MainInput)
	{
		//need to disconnect our own inputs
		//auto& BuilderSubsystems = SequencerData->MSBuilderSystem;
		auto& BuilderContext = SequencerData->BuilderContext;

		if(!BuilderContext)
		{
			UE_LOG(unDAWVertexLogs, VeryVerbose, TEXT("Builder Context is null!"))
			return;
		}

		EMetaSoundBuilderResult BuildResult;
		BuilderContext->DisconnectNodeInput(AudioOutput.AudioLeftOutputInputHandle, BuildResult);
		BuilderConnectionResults.Add(FName(TEXT("Disconnect Audio Stream L")), BuildResult);
		BuilderContext->DisconnectNodeInput(AudioOutput.AudioRightOutputInputHandle, BuildResult);
		BuilderConnectionResults.Add(FName(TEXT("Disconnect Audio Stream R")), BuildResult);
				
		UE_LOG(unDAWVertexLogs, VeryVerbose, TEXT("Main Input is null!"))
		return;
	}

	//otherwise, well, we have an AssignedOutput, we need to find the audio streams of the upstream vertex, connect them and that's it, show me what you got co-pilot

	auto& BuilderSubsystems = SequencerData->MSBuilderSystem;
	auto& BuilderContext = SequencerData->BuilderContext;
	EMetaSoundBuilderResult BuildResult;

	//find the audio stream outputs of the main input using the AutoConnectOutPins
	auto UpStreamLeftAudio = MainInput->AutoConnectOutPins.Find(EVertexAutoConnectionPinCategory::AudioStreamL);
	auto UpStreamRightAudio = MainInput->AutoConnectOutPins.Find(EVertexAutoConnectionPinCategory::AudioStreamR);

	//connect the audio streams to the audio output node
	bool Success = false;
	if (UpStreamLeftAudio)
	{
		BuilderContext->ConnectNodes(*UpStreamLeftAudio, AudioOutput.AudioLeftOutputInputHandle, BuildResult);
		BuilderResults.Add(FName(TEXT("Connect Audio Stream L to Audio Output")), BuildResult);
		Success = ResultToBool(BuildResult);
	}

	if (UpStreamRightAudio)
	{
		BuilderContext->ConnectNodes(*UpStreamRightAudio, AudioOutput.AudioRightOutputInputHandle, BuildResult);
		BuilderResults.Add(FName(TEXT("Connect Audio Stream R to Audio Output")), BuildResult);
		Success = ResultToBool(BuildResult) && Success;
	}

	if(Success)
	{
		BuilderConnectionResults.Add(FName(TEXT("Audio Output Connection Success")), EMetaSoundBuilderResult::Succeeded);
	}
	else
	{
		BuilderConnectionResults.Add(FName(TEXT("Audio Output Connection Failed")), EMetaSoundBuilderResult::Failed);
	}


}

void UM2SoundAudioOutput::CollectAndTransmitAudioParameters()
{
	//this is a test, we just need to transmit the gain parameter
	TransmitAudioParameter(FAudioParameter(AudioOutput.OutputName, Gain));
}


void UM2SoundBuilderInputHandleNode::BuildVertex()
{
	EMetaSoundBuilderResult BuildResult;
	BuilderResults.Empty();
	auto& BuilderSubsystems = SequencerData->MSBuilderSystem;
	auto& BuilderContext = SequencerData->BuilderContext;

	//we should actually only clear the outputs if we're initializing a node, otherwsie we may want to preserve the connections according to the metasound semantics
	MetasoundOutputs.Empty();
	auto ChannelFilterNode = BuilderContext->AddNode(SequencerData->CoreNodes.MidiFilterDocument, BuildResult);
	auto NodeHandle = ChannelFilterNode;
	BuilderResults.Add(FName(TEXT("Add Midi Filter Metasound Node")), BuildResult);

	InPins = BuilderContext->FindNodeInputs(ChannelFilterNode, BuildResult);
	OutPins = BuilderContext->FindNodeOutputs(ChannelFilterNode, BuildResult);

	//make static connections - need to think again about this
	auto TrackInput = BuilderContext->FindNodeInputByName(ChannelFilterNode, FName(TEXT("Track")), BuildResult);
	FName MetasoundIntDatatypeName = TEXT("int32");
	auto& TrackMetadata = SequencerData->GetTracksDisplayOptions(TrackId);
	auto TrackInputNodeOutput = BuilderContext->AddGraphInputNode(FName(TrackPrefix + TEXT("TrackNum")), TEXT("int32"), BuilderSubsystems->CreateIntMetaSoundLiteral(TrackMetadata.TrackIndexInParentMidi, MetasoundIntDatatypeName), BuildResult);
	BuilderContext->ConnectNodes(TrackInputNodeOutput, TrackInput, BuildResult);

	BuilderResults.Add(FName(TEXT("Midi Track Filter Num Assignment")), BuildResult);

	auto ChannelInput = BuilderContext->FindNodeInputByName(ChannelFilterNode, FName(TEXT("Channel")), BuildResult);
	auto ChannelInputNodeOutput = BuilderContext->AddGraphInputNode(FName(TrackPrefix + TEXT("Channel")), TEXT("int32"), BuilderSubsystems->CreateIntMetaSoundLiteral(TrackMetadata.ChannelIndexInParentMidi, MetasoundIntDatatypeName), BuildResult);
	BuilderContext->ConnectNodes(ChannelInputNodeOutput, ChannelInput, BuildResult);

	BuilderResults.Add(FName(TEXT("Midi Ch. Filter Num Assignment")), BuildResult);

	//connect to midi player midistream output, this is actually the most important part!
	auto MidiInput = BuilderContext->FindNodeInputByName(ChannelFilterNode, FName(TEXT("MIDI Stream")), BuildResult);
	BuilderContext->ConnectNodes(SequencerData->CoreNodes.MainMidiStreamOutput, MidiInput, BuildResult);

	BuilderResults.Add(FName(TEXT("Connect to MIDI Stream")), BuildResult);

	auto NewNodeMidiStreamOutput = BuilderContext->FindNodeOutputByName(ChannelFilterNode, FName(TEXT("MIDI Stream")), BuildResult);

	AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::MidiTrackStream, NewNodeMidiStreamOutput);
	BuilderResults.Add(FName(TEXT("Expose Auto Connect Midi Stream MetaPin")), BuildResult);

	auto NewNodeTrackOutput = BuilderContext->FindNodeOutputByName(ChannelFilterNode, FName(TEXT("Track")), BuildResult);
	AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::MidiTrackTrackNum, NewNodeTrackOutput);
	BuilderResults.Add(FName(TEXT("Expose Auto Connect Track MetaPin")), BuildResult);

}

void UM2SoundPatch::BuildVertex()
{
	BuilderResults.Empty();
	EMetaSoundBuilderResult BuildResult;
	auto& BuilderSubsystems = SequencerData->MSBuilderSystem;
	auto& BuilderContext = SequencerData->BuilderContext;


	NodeHandle = BuilderContext->AddNode(Patch, BuildResult);
	BuilderResults.Add(FName(TEXT("Add Patch Node")), BuildResult);
	//OutBuiltData.NodeHandle = NodeHandle;
	BuilderResults.Add(FName(TEXT("Add Patch Node")), BuildResult);
	InPins = BuilderContext->FindNodeInputs(NodeHandle, BuildResult);
	OutPins = BuilderContext->FindNodeOutputs(NodeHandle, BuildResult);
}

void UM2SoundPatch::UpdateConnections()
{
	BuilderConnectionResults.Empty();


	if (!MainInput)
	{
		UE_LOG(unDAWVertexLogs, Log, TEXT("Main Input is null!"))
			return;
	}

	//dilligently we ought to check for our interfaces, but, lazy, we'll actually use the builder context to find the inputs and outputs
	auto& BuilderSubsystems = SequencerData->MSBuilderSystem;
	auto& BuilderContext = SequencerData->BuilderContext;

	EMetaSoundBuilderResult BuildResult;

	auto MidiStreamInName = FName(TEXT("unDAW Instrument.MidiStream"));
	auto MidiTrackInName = FName(TEXT("unDAW Instrument.MidiTrack"));

	for (auto& [Type, Input] : AutoConnectInPins)
	{
		switch (Type)
		{
			case EVertexAutoConnectionPinCategory::MidiTrackStream:
			{
						auto UpStreamConnection = MainInput->AutoConnectOutPins.Find(EVertexAutoConnectionPinCategory::MidiTrackStream);
						if (UpStreamConnection)
						{
							BuilderContext->ConnectNodes(*UpStreamConnection, Input, BuildResult);
							BuilderConnectionResults.Add(MidiStreamInName, BuildResult);
						}
						else {
							UE_LOG(unDAWVertexLogs, Error, TEXT("Main Input does not have a Midi Stream Output, yet our node is an instrumenet renderer"))
						}

						break;

			}

			case EVertexAutoConnectionPinCategory::MidiTrackTrackNum:
			{
						auto UpStreamConnection = MainInput->AutoConnectOutPins.Find(EVertexAutoConnectionPinCategory::MidiTrackTrackNum);
						if (UpStreamConnection)
						{
							BuilderContext->ConnectNodes(*UpStreamConnection, Input, BuildResult);
							BuilderConnectionResults.Add(MidiTrackInName, BuildResult);
						}
						else {
							UE_LOG(unDAWVertexLogs, Error, TEXT("Main Input does not have a Midi Track Output, yet our node is an instrumenet renderer"))
						}

						break;
			}

			case EVertexAutoConnectionPinCategory::AudioStreamL:
			{
						auto UpStreamConnection = MainInput->AutoConnectOutPins.Find(EVertexAutoConnectionPinCategory::AudioStreamL);
						if (UpStreamConnection)
						{
							BuilderContext->ConnectNodes(*UpStreamConnection, Input, BuildResult);
							BuilderConnectionResults.Add(FName(TEXT("Audio In L")), BuildResult);
						}
						else {
							UE_LOG(unDAWVertexLogs, Error, TEXT("Main Input does not have a Audio Stream L Output, yet our node is an insert"))
						}

						break;
			}

			case EVertexAutoConnectionPinCategory::AudioStreamR:
			{
						auto UpStreamConnection = MainInput->AutoConnectOutPins.Find(EVertexAutoConnectionPinCategory::AudioStreamR);
						if (UpStreamConnection)
						{
							BuilderContext->ConnectNodes(*UpStreamConnection, Input, BuildResult);
							BuilderConnectionResults.Add(FName(TEXT("Audio In R")), BuildResult);
						}
						else {
							UE_LOG(unDAWVertexLogs, Error, TEXT("Main Input does not have a Audio Stream R Output, yet our node is an insert"))
						}

						break;
			}

		}

	}


	auto MidiStreamInput = BuilderContext->FindNodeInputByName(NodeHandle, MidiStreamInName, BuildResult);
	switch (BuildResult)
	{
	case EMetaSoundBuilderResult::Succeeded:
		UE_LOG(unDAWVertexLogs, Log, TEXT("Found Midi Stream Input"))
			if (MainInput->AutoConnectOutPins.Contains(EVertexAutoConnectionPinCategory::MidiTrackStream))
			{
			auto& InputVertexMidiOut = MainInput->AutoConnectOutPins[EVertexAutoConnectionPinCategory::MidiTrackStream];
			BuilderContext->ConnectNodes(InputVertexMidiOut, MidiStreamInput, BuildResult);
			BuilderConnectionResults.Add(MidiStreamInName, BuildResult);
			}
			else {
				UE_LOG(unDAWVertexLogs, Error, TEXT("Main Input does not have a Midi Stream Output, yet our node is an instrumenet renderer"))
			}

		break;

	case EMetaSoundBuilderResult::Failed:
		UE_LOG(unDAWVertexLogs, VeryVerbose, TEXT("Failed to find Midi Stream Input, we may be an insert"))
		break;
	}
	
	auto MidiTrackInput = BuilderContext->FindNodeInputByName(NodeHandle, MidiTrackInName, BuildResult);
	switch (BuildResult)
	{
	case EMetaSoundBuilderResult::Succeeded:
		UE_LOG(unDAWVertexLogs, Log, TEXT("Found Midi Track Input"))
			if (MainInput->AutoConnectOutPins.Contains(EVertexAutoConnectionPinCategory::MidiTrackTrackNum))
			{
				auto& InputVertexTrackOut = MainInput->AutoConnectOutPins[EVertexAutoConnectionPinCategory::MidiTrackTrackNum];
				BuilderContext->ConnectNodes(InputVertexTrackOut, MidiTrackInput, BuildResult);
				BuilderConnectionResults.Add(MidiTrackInName, BuildResult);
			}
			else {
				UE_LOG(unDAWVertexLogs, Error, TEXT("Main Input does not have a Midi Track Output, yet our node is an instrumenet renderer"))
			}

		break;

	case EMetaSoundBuilderResult::Failed:
		UE_LOG(unDAWVertexLogs, VeryVerbose, TEXT("Failed to find Midi Track Input, we may be an insert"))
			break;

	}


	

}
