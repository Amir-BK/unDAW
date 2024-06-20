#include "Vertexes/M2SoundVertex.h"
#include "M2SoundGraphRenderer.h"
#include "M2SoundGraphStatics.h"
#include "Metasound.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"

#include "M2SoundGraphRenderer.h"
#include "unDAWSettings.h"

DEFINE_LOG_CATEGORY(unDAWVertexLogs);

void UM2SoundPatch::SaveDefaultsToVertexCache()
{
	auto Config = FCachedVertexPinInfo();

	for (const auto& [Name, Pin] : InPinsNew)
	{
		Config.PinRanges.Add(Name, FFloatRange(Pin.MinValue, Pin.MaxValue));
	}

	
	UUNDAWSettings::Get()->Cache.Add(Patch->GetFName(), Config);
	UUNDAWSettings::Get()->SaveConfig();
}

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

void UM2SoundVertex::UpdateValueForPin(FM2SoundPinData& Pin, FMetasoundFrontendLiteral& NewValue)
{
	//in theory we should should check whether this pin has a param connected... but we're just going to set the value for now
	
	EMetaSoundBuilderResult BuildResult;
	GetSequencerData()->BuilderContext->SetNodeInputDefault(Pin.InputHandle, NewValue, BuildResult);

	if(BuildResult == EMetaSoundBuilderResult::Failed)	BuilderConnectionResults.Add(Pin.PinName, BuildResult);
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

	//need to inform downstream vertexes to reconnect to our output
	for (auto Output : Outputs)
	{
		Output->UpdateConnections();
	}
	OnVertexUpdated.Broadcast();
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

#define TOFLAG(Enum) (1 << static_cast<uint8>(Enum))

void UM2SoundVertex::CollectParamsForAutoConnect()
{
	//called right after the vertex is created, we should collect the parameters that are exposed by the vertex and can be auto connected
	//to other nodes in the metasound graph
	AutoConnectInPins.Empty();
	AutoConnectOutPins.Empty();
	//Pins.Empty(); //probably don't want to clear the pins but we're just running a quick test now
	//make temp copies of the in and out pins
	//TMap<FName, FM2SoundPinData> InPinsNew;
	//TMap<FName, FM2SoundPinData> OutPinsNew;

	auto CopyOfInPins = InPinsNew;


	InPinsNew.Empty();
	OutPinsNew.Empty();

	
	EMetaSoundBuilderResult BuildResult;

	auto& BuilderSubsystems = SequencerData->MSBuilderSystem;
	auto& BuilderContext = SequencerData->BuilderContext;

	TryFindVertexDefaultRangesInCache();

	//find the inputs and outputs of the node
	//InPins = BuilderContext->FindNodeInputs(NodeHandle, BuildResult);
	for(const auto& Input : InPins)
	{
		
		
		
		FM2SoundPinData PinData;
				
		FName PinName;
		FName DataType;
		BuilderContext->GetNodeInputData(Input, PinName, DataType, BuildResult);
		auto LiteralValue = BuilderContext->GetNodeInputClassDefault(Input, BuildResult);
		

		if (CopyOfInPins.Contains(PinName))
		{
			PinData = CopyOfInPins[PinName];
			//set the existing literal as default for the input handle
			BuilderContext->SetNodeInputDefault(Input, PinData.LiteralValue, BuildResult);
		}
		else
		{
			
			//if we're a patchvertex...

			//if we're a patch vertex, we should check the cache for the default values of the pins
			//if we're not a patch vertex, it doesn't matter

			UM2SoundPatch* Patch = Cast<UM2SoundPatch>(this);
			FM2SoundPinData* CachedPinData = nullptr;

			if (Patch)
			{
				auto& Cache = UUNDAWSettings::Get()->Cache;


				if (Cache.Contains(Patch->Patch->GetFName()))
				{
					auto& CachedVertexInfo = Cache[Patch->Patch->GetFName()];
					if (CachedVertexInfo.PinRanges.Contains(PinName))
					{
						auto& CachedPin = CachedVertexInfo.PinRanges[PinName];
						PinData.MinValue = CachedPin.GetLowerBoundValue();
						PinData.MaxValue = CachedPin.GetUpperBoundValue();
						//PinData.LiteralValue = BuilderContext->CreateFloatMetaSoundLiteral(CachedPin.GetLowerBound(), DataType);
						UE_LOG(unDAWVertexLogs, Verbose, TEXT("Found Cached Pin %s"), *PinName.ToString())
						CachedPinData = &PinData;
					}
				}

			}


			if (CachedPinData)
			{
				PinData = *CachedPinData;
			}
			else {
			PinData = FM2SoundPinData();

			}
			
			
			PinData.PinName = PinName;
			PinData.DataType = DataType;
			if (DataType == FName(TEXT("Float")))
			{
				UE_LOG(unDAWVertexLogs, Verbose, TEXT("Found Float Pin %s"), *PinName.ToString())
				PinData.DisplayFlags |= static_cast<uint8>(EM2SoundPinDisplayFlags::ShowInGraph);

				float TryGetValue;
				if(LiteralValue.TryGet(TryGetValue))
				{
					PinData.NormalizedValue = FMath::GetRangePct(PinData.MinValue, PinData.MaxValue, TryGetValue);
					//also print try get value...
					UE_LOG(unDAWVertexLogs, Verbose, TEXT("Got float value from literal %f"), TryGetValue)
				}
				else
				{
					UE_LOG(unDAWVertexLogs, Error, TEXT("Failed to get float value from literal for float pin"))
				}


			}
			PinData.LiteralValue = LiteralValue;
			BuilderConnectionResults.Add(FName(TEXT("Get Default Value")), BuildResult);
		}

		bool IsAutoManaged = false;
		PinData.InputHandle = Input;
		




		if(PinName == FName(TEXT("unDAW Instrument.MidiStream")))
		{
			IsAutoManaged = true;
			AutoConnectInPins.Add(EVertexAutoConnectionPinCategory::MidiTrackStream, Input);
		}
		
		if (PinName == FName(TEXT("unDAW Instrument.MidiTrack")))
		{
			AutoConnectInPins.Add(EVertexAutoConnectionPinCategory::MidiTrackTrackNum, Input);
			//PinData.PinFlags |=  TOFLAG(EM2SoundPinFlags::IsAutoManaged);
			IsAutoManaged = true;
		}

		//also check the insert names unDAW Insert.Audio In L etc
		if(PinName.ToString().Contains(TEXT("unDAW Insert.Audio In L")))
		{
			AutoConnectInPins.Add(EVertexAutoConnectionPinCategory::AudioStreamL, Input);
			//PinData.PinFlags |= EM2SoundPinFlags::IsAutoManaged;
			IsAutoManaged = true;
		}

		if(PinName.ToString().Contains(TEXT("unDAW Insert.Audio In R")))
		{
			AutoConnectInPins.Add(EVertexAutoConnectionPinCategory::AudioStreamR, Input);
			//PinData.PinFlags |=  TOFLAG(EM2SoundPinFlags::IsAutoManaged);
			IsAutoManaged = true;
		}

		if(IsAutoManaged)
		{
			//PinData.PinTypeFlags |= EM2SoundPinFlags::IsAutoManaged;
			//EnumAddFlags(PinData.PinTypeFlags, EM2SoundPinFlags::IsAutoManaged & EM2SoundPinFlags::IsConnectedToGraphParam);
			PinData.PinFlags |= static_cast<uint8>(EM2SoundPinFlags::IsAutoManaged);
			//PinData.PinFlags << EM2SoundPinFlags::IsAutoManaged;
		}

		InPinsNew.Add(PinName, PinData);

	}

	for (const auto& Output : OutPins)
	{
		FM2SoundPinData PinData = FM2SoundPinData();

		FName& NodeName = PinData.PinName;
		FName& DataType = PinData.DataType;
		bool IsAutoManaged = false;
		BuilderContext->GetNodeOutputData(Output, NodeName, DataType, BuildResult);

		if (NodeName == FName(TEXT("unDAW Instrument.Audio L")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::AudioStreamL, Output);
			IsAutoManaged = true;
		}

		if (NodeName == FName(TEXT("unDAW Instrument.Audio R")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::AudioStreamR, Output);
			IsAutoManaged = true;
		}

		if (NodeName == FName(TEXT("MIDI Stream")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::MidiTrackStream, Output);
			IsAutoManaged = true;

	
		}

		if (NodeName == FName(TEXT("Track")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::MidiTrackTrackNum, Output);
			IsAutoManaged = true;


		}

		//insert audio outputs unDAW Insert.Audio L 

		if (NodeName == FName(TEXT("unDAW Insert.Audio L")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::AudioStreamL, Output);
			IsAutoManaged = true;


		}

		if (NodeName == FName(TEXT("unDAW Insert.Audio R")))
		{
			AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::AudioStreamR, Output);
			IsAutoManaged = true;


		}

		if (IsAutoManaged)
		{
			//PinData.PinTypeFlags |= EM2SoundPinFlags::IsAutoManaged;
			PinData.PinFlags |= static_cast<uint8>(EM2SoundPinFlags::IsAutoManaged);
		}

		OutPinsNew.Add(NodeName, PinData);

	}

	OnVertexUpdated.Broadcast();

}

#if WITH_EDITOR

void UM2SoundVertex::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	//VertexNeedsBuilderUpdates();
	//let's just do a quick test, print param name
	//UE_LOG(unDAWVertexLogs, Verbose, TEXT("PostEditChangeProperty %s"), *GetName())
	auto Property = PropertyChangedEvent.Property;
	auto PropertyName = Property->GetFName();

	if (PropertyName == FName(TEXT("DisplayFlags")))
	{
		OnVertexUpdated.Broadcast();
	}

	if (PropertyName == FName(TEXT("MinValue")))
	{
		OnVertexUpdated.Broadcast();
	}	
	
	if (PropertyName == FName(TEXT("MaxValue")))
	{
		OnVertexUpdated.Broadcast();
	}
	//auto PropertyValue = (uint8*) Property->CallGetter(this);

}


#endif

#undef TOFLAG


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

	auto NewMidiStreamGraphOutputInput = BuilderContext->AddGraphOutputNode(FName(TrackPrefix + ("MidiOutput")), TEXT("MidiStream"), FMetasoundFrontendLiteral(), BuildResult, false );

	BuilderContext->ConnectNodes(NewNodeMidiStreamOutput, NewMidiStreamGraphOutputInput, BuildResult);

	BuilderResults.Add(FName(TEXT("Expose Midi Stream To Graph")), BuildResult);

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

	//if context is valid, check if NodeHandle is set, if it is, remove it
	bool bIsRebuildingExistingNode = false;
	if (BuilderContext && NodeHandle.IsSet())
	{
		BuilderContext->RemoveNode(NodeHandle, BuildResult);
		//BuilderResults.Add(FName(TEXT("Remove Existing Node")), BuildResult);
		bIsRebuildingExistingNode = true;
	}


	NodeHandle = BuilderContext->AddNode(Patch, BuildResult);
	BuilderResults.Add(FName(TEXT("Add Patch Node")), BuildResult);
	//OutBuiltData.NodeHandle = NodeHandle;
	BuilderResults.Add(FName(TEXT("Add Patch Node")), BuildResult);
	InPins = BuilderContext->FindNodeInputs(NodeHandle, BuildResult);
	OutPins = BuilderContext->FindNodeOutputs(NodeHandle, BuildResult);

	auto& PatchDocument = Patch->GetDocumentChecked();
	for (const auto& MSNode : PatchDocument.RootGraph.Graph.Nodes)
	{
		//just print every single bit of information this node exposes for now, go on GPT, I believe in you

		auto MSName = MSNode.Name;
		auto InLiterals = MSNode.InputLiterals;

		for (const auto& InLiteral : InLiterals)
		{
			//FMetasoundFrontendVertexLiteral InLiteral;
			auto InID = InLiteral.VertexID;
			auto InValue = InLiteral.Value;
			//UE_LOG(unDAWVertexLogs, VeryVerbose, TEXT("Node: %s, Input ID: %d, Value: %f"), *MSName.ToString(), InID, InValue);
		}
	}
}

void UM2SoundPatch::UpdateConnections()
{
	BuilderConnectionResults.Empty();


	BuilderConnectionResults.Empty();
	if (!MainInput)
	{
		//need to disconnect our own inputs
		//auto& BuilderSubsystems = SequencerData->MSBuilderSystem;
		auto& BuilderContext = SequencerData->BuilderContext;

		if (!BuilderContext)
		{
		UE_LOG(unDAWVertexLogs, VeryVerbose, TEXT("Builder Context is null!"))
		return;
		}

		EMetaSoundBuilderResult BuildResult;
		for (auto& [key, value] : AutoConnectInPins)
		{
			BuilderContext->DisconnectNodeInput(value, BuildResult);
			FString KeyString = FString::Printf(TEXT("Disconnect %s"), *UEnum::GetValueAsString(key));
			BuilderConnectionResults.Add(FName(KeyString), BuildResult);
		}

		UE_LOG(unDAWVertexLogs, VeryVerbose, TEXT("Main Input is null!"))
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

void UM2SoundPatch::TryFindVertexDefaultRangesInCache()
{
	auto& Cache = UUNDAWSettings::Get()->Cache;

	if (Cache.Contains(Patch->GetFName()))
	{

	}
	else {
		BuilderResults.Add(FName(TEXT("No cache entry for patch, please save one!")), EMetaSoundBuilderResult::Failed);
	}
	
}
