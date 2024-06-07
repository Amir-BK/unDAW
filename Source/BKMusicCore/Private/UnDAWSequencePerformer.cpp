// Fill out your copyright notice in the Description page of Project Settings.


#include "UnDAWSequencePerformer.h"
#include "Engine/Engine.h"
#include "IAudioParameterInterfaceRegistry.h"
#include "Kismet/GameplayStatics.h"

#include "MetasoundGeneratorHandle.h"
#include "MetasoundGenerator.h"
#include "MetasoundAssetSubsystem.h"
#include "MetasoundAssetBase.h"
#include "MetasoundOutputSubsystem.h"
#include "MetasoundFrontendSearchEngine.h"
#include "M2SoundGraphStatics.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"


UM2SoundGraphRenderer::~UM2SoundGraphRenderer()
{

	if (AuditionComponentRef)
	{
		AuditionComponentRef->Stop();
		AuditionComponentRef->DestroyComponent();
	}



}



void UM2SoundGraphRenderer::SendTransportCommand(EBKTransportCommands Command)
{
	if (AuditionComponentRef)
	{
		switch (Command)
		{

		case EBKTransportCommands::Play:
			AuditionComponentRef->SetTriggerParameter(FName(TEXT("unDAW.Transport.Play")));
			PlayState = EBKPlayState::Playing;
			break;

		case EBKTransportCommands::Stop:
			AuditionComponentRef->SetTriggerParameter(FName(TEXT("unDAW.Transport.Stop")));
			PlayState = EBKPlayState::ReadyToPlay;

			break;

		case EBKTransportCommands::Pause:
		AuditionComponentRef->SetTriggerParameter(FName(TEXT("unDAW.Transport.Pause")));
			PlayState = EBKPlayState::Paused;
			break;
		default:
			break;


		}
	}
}

void UM2SoundGraphRenderer::InitPerformer()
{

	//outer must be dawsequencerdata
	SessionData = CastChecked<UDAWSequencerData>(GetOuter());

	MSBuilderSystem = GEngine->GetEngineSubsystem<UMetaSoundBuilderSubsystem>();
	EMetaSoundBuilderResult BuildResult;

	FMetaSoundBuilderNodeInputHandle OnFinished;

	CurrentBuilder = MSBuilderSystem->CreateSourceBuilder(FName("BuilderName-unDAW Session Rednerer"), OnPlayOutputNode, OnFinished, AudioOuts, BuildResult, SessionData->MasterOptions.OutputFormat, false);

	SessionData->OnVertexAdded.AddDynamic(this, &UM2SoundGraphRenderer::UpdateVertex);

	//after builder is created, create the nodes and connections

	//create renderer interface default IOs
	CurrentBuilder->AddInterface(FName(TEXT("unDAW Session Renderer")), BuildResult);

	//add main clock patch by reference - '/Script/MetasoundEngine.MetaSoundPatch'/unDAW/Patches/System/unDAW_MainClock.unDAW_MainClock'
	CreateMidiPlayerBlock();

	FSoftObjectPath MidiFilterPatchSoftPath(TEXT("/unDAW/Patches/System/unDAW_MidiFilter.unDAW_MidiFilter"));
	MidiFilterDocument = MidiFilterPatchSoftPath.TryLoad();
	//CurrentBuilder->AddNode(MidiFilterDocument, BuildResult);
	//UMetaSoundPatch* MidiPatch = Cast<UMetaSoundPatch>(MidiPlayerDocument.GetObject());

	CreateMixerNodesSpaghettiBlock();


	TArray<UM2SoundVertex*> AllVertices = UM2SoundGraphStatics::GetAllVertexesInSequencerData(SessionData);

	for (auto& [index, vertex] : SessionData->TrackInputs)
	{
		UpdateVertex(vertex);

	}

	//update patches
	
	for (auto& [index, vertex] : SessionData->Patches)
	{
		UpdateVertex(vertex);
	
	}

	// update outputs
	for (auto& [index, vertex] : SessionData->Outputs)
	{
		UpdateVertex(vertex);
		
	}
	
	// this won't work, we need to traverse the vertices from the inputs to the outputs
	//for(const auto& Vertex : AllVertices)
	//{
	//	UpdateVertex(Vertex);
	//	Vertex->OnVertexNeedsBuilderUpdates.AddDynamic(this, &UDAWSequencerPerformer::UpdateVertex);
	//}

}

FM2SoundMetasoundBuilderPinData CreatePinDataFromBuilderData(UMetaSoundSourceBuilder* Builder, FMetaSoundBuilderNodeInputHandle Input, EMetaSoundBuilderResult& BuildResult)
{
	FName NodeName;
	FName DataType;
	Builder->GetNodeInputData(Input, NodeName, DataType, BuildResult);
	FM2SoundMetasoundBuilderPinData PinData;
	PinData.PinName = NodeName;
	PinData.DataType = DataType;
	return PinData;
}

void UM2SoundGraphRenderer::UpdateVertex(UM2SoundVertex* Vertex)
{
	EMetaSoundBuilderResult BuildResult;
	
	//clear vertex build results
	Vertex->BuilderResults.Empty();

	//UE_LOG(LogTemp, Log, TEXT("Vertex Name: %s"), *Vertex->GetName())
	//Vertex->GetDocumentChecked().GetDocumentName();
	if (UM2SoundTrackInput* InputVertex = Cast<UM2SoundTrackInput>(Vertex))
	{
		//create track filter node
		InputVertex->MetasoundOutputs.Empty();
		//add node by patch reference - '/unDAW/Patches/System/unDAW_MidiFilter.unDAW_MidiFilter'
		auto ChannelFilterNode = CurrentBuilder->AddNode(MidiFilterDocument, BuildResult);
		InputVertex->BuilderResults.Add(FName("Main Node"), BuildResult);

		//connect input of filter node to midi player node stream output

		//add filter output to vertex outputs
		auto FilterOutputs = CurrentBuilder->FindNodeOutputs(ChannelFilterNode, BuildResult);
		for (const auto& Output : FilterOutputs)
		{
			FName OutputName;
			FName DataType;
			CurrentBuilder->GetNodeOutputData(Output, OutputName, DataType, BuildResult);
			InputVertex->MetasoundOutputs.Add(FM2SoundMetasoundBuilderPinData{ OutputName, DataType });
			InputVertex->MidiStreamOutput = Output;
			//Connect the midi stream output to the midi player node
			InputVertex->BuilderResults.Add(OutputName, BuildResult);

			//hacky but we assume only one output here
		}

		InputVertex->TrackPrefix = FString::Printf(TEXT("Tr%d_Ch%d."), InputVertex->TrackId, InputVertex->MidiChannel);

		//find midi input and connect it to main midi player
		auto MidiInput = CurrentBuilder->FindNodeInputByName(ChannelFilterNode, FName(TEXT("MIDI Stream")), BuildResult);

		//find int input named "track" and assign its default value to the vertex's track id
		auto TrackInput = CurrentBuilder->FindNodeInputByName(ChannelFilterNode, FName(TEXT("Track")), BuildResult);
		FName intDataType = TEXT("int32");

		auto TrackInputNodeOutput = CurrentBuilder->AddGraphInputNode(FName(InputVertex->TrackPrefix + TEXT("TrackNum")), TEXT("int32"), MSBuilderSystem->CreateIntMetaSoundLiteral(InputVertex->TrackId, intDataType), BuildResult);
		CurrentBuilder->ConnectNodes(TrackInputNodeOutput, TrackInput, BuildResult);

		//same for "channel"
		auto ChannelInput = CurrentBuilder->FindNodeInputByName(ChannelFilterNode, FName(TEXT("Channel")), BuildResult);
		auto ChannelInputNodeOutput = CurrentBuilder->AddGraphInputNode(FName(InputVertex->TrackPrefix + TEXT("Channel")), TEXT("int32"), MSBuilderSystem->CreateIntMetaSoundLiteral(InputVertex->MidiChannel, intDataType), BuildResult);
		CurrentBuilder->ConnectNodes(ChannelInputNodeOutput, ChannelInput, BuildResult);
		//add to build results
		InputVertex->BuilderResults.Add(FName(TEXT("Track Connection")), BuildResult);


		CurrentBuilder->ConnectNodes(MainMidiStreamOutput, MidiInput, BuildResult);
		InputVertex->BuilderResults.Add(FName(TEXT("Connect to main player")), BuildResult);
		
		//CreateDefaultVertexesFromInputVertex(InSessionData, InputVertex, InputVertex->TrackId);
	}

	if (UM2SoundPatch* PatchVertex = Cast<UM2SoundPatch>(Vertex))
	{
		//check if we already have a node and if so, delete it
		if (VertexToNodeMap.Contains(PatchVertex))
		{
			auto NodeHandle = VertexToNodeMap[PatchVertex];
			CurrentBuilder->RemoveNode(NodeHandle, BuildResult);
			VertexToNodeMap.Remove(PatchVertex);
		}

		TScriptInterface<IMetaSoundDocumentInterface> asDocInterface = PatchVertex->Patch;
		auto NewNodeHandle = CurrentBuilder->AddNode(asDocInterface, BuildResult);
		auto NodeInputs = CurrentBuilder->FindNodeInputs(NewNodeHandle, BuildResult);
		auto NodeOutputs = CurrentBuilder->FindNodeOutputs(NewNodeHandle, BuildResult);

		// clear vertex discovered I/Os

		//in theory, if we empty these and then recreate I/Os the I/Os that maintain their names can be reconnected
		PatchVertex->MetasoundInputs.Empty();
		PatchVertex->MetasoundOutputs.Empty();
		
		for( const auto& Input : NodeInputs)
		{
			FName NodeName;
			FName DataType;
			CurrentBuilder->GetNodeInputData(Input, NodeName, DataType, BuildResult);
			PatchVertex->MetasoundInputs.Add(FM2SoundMetasoundBuilderPinData{ NodeName, DataType });

		}

		//find the two audio outputs and connect them to the available audio outputs

		auto FreeOutputs = GetFreeAudioOutput();

		for (const auto& Output : NodeOutputs)
		{

			FName OutputName;
			FName DataType;
			CurrentBuilder->GetNodeOutputData(Output, OutputName, DataType, BuildResult);

			if(DataType == FName(TEXT("Audio")))
			{
				CurrentBuilder->ConnectNodes(Output, FreeOutputs.Pop(), BuildResult);
				PatchVertex->BuilderResults.Add(OutputName, BuildResult);
			}

			CurrentBuilder->GetNodeOutputData(Output, OutputName, DataType, BuildResult);
			PatchVertex->MetasoundOutputs.Add(FM2SoundMetasoundBuilderPinData{ OutputName, DataType });
		}
		
		VertexToNodeMap.Add(PatchVertex, NewNodeHandle);

		if(PatchVertex->Inputs.Num() > 0)
		{
			//fusion patch reference for patch node
			auto PatchForLiteral = SessionData->GetTracksDisplayOptions(1).fusionPatch.Get();
			
			auto PatchInput = CurrentBuilder->FindNodeInputByName(NewNodeHandle, FName(TEXT("Patch")), BuildResult);
			auto PatchInputNodeOutput = CurrentBuilder->AddGraphInputNode(TEXT("Patch"), TEXT("FusionPatchAsset"), MSBuilderSystem->CreateObjectMetaSoundLiteral(PatchForLiteral), BuildResult);
			CurrentBuilder->ConnectNodes(PatchInputNodeOutput, PatchInput, BuildResult);

			//connect this instrument renderer to the midi stream output by vertex connection
			auto MidiStreamInput = CurrentBuilder->FindNodeInputByName(NewNodeHandle, FName(TEXT("unDAW Instrument.MidiStream")), BuildResult);
			auto AsInputVertex = Cast<UM2SoundTrackInput>(PatchVertex->Inputs[0]);

			//find interface track num input
			auto TrackInput = CurrentBuilder->FindNodeInputByName(NewNodeHandle, FName(TEXT("unDAW Instrument.MidiTrack")), BuildResult);
			//find the track number input via input vertex, it should already exist, use the track prefix from the input note
			//auto TrackNumGraphInput = CurrentBuilder->FindGraphInputNode(, BuildResult);
			CurrentBuilder->ConnectNodeInputToGraphInput(FName(AsInputVertex->TrackPrefix + TEXT("TrackNum")), TrackInput, BuildResult);
			//add to build results
			PatchVertex->BuilderResults.Add(FName(TEXT("Track Connection")), BuildResult);



			CurrentBuilder->ConnectNodes(AsInputVertex->MidiStreamOutput, MidiStreamInput, BuildResult);
			//add to build results
			PatchVertex->BuilderResults.Add(FName(TEXT("Connect to main player")), BuildResult);

		}

		//CreateDefaultVertexesFromInputVertex(InSessionData, PatchVertex, PatchVertex->TrackId);
		
	}
	
	if (UM2SoundMidiOutput* MidiOutputVertex = Cast<UM2SoundMidiOutput>(Vertex))
	{
		//CreateDefaultVertexesFromInputVertex(InSessionData, MidiOutputVertex, MidiOutputVertex->TrackId);
	}

	Vertex->OnVertexUpdated.Broadcast();
	Vertex->OnVertexNeedsBuilderUpdates.AddDynamic(this, &UM2SoundGraphRenderer::UpdateVertex);

}

void UM2SoundGraphRenderer::CreateAuditionableMetasound(UAudioComponent* InComponent, bool bReceivesLiveUpdates)
{

	if(!CurrentBuilder)
	{
		//This should never be called, a builder must be set up via InitPerformer prior to attempting to create a live audition component
		checkNoReentry();
	}
	
	if (AuditionComponentRef)
	{
		AuditionComponentRef->Stop();
		AuditionComponentRef->DestroyComponent();
	}


	AuditionComponentRef = InComponent;


	PlayState = EBKPlayState::ReadyToPlay;
	FOnCreateAuditionGeneratorHandleDelegate OnCreateAuditionGeneratorHandle;
	OnCreateAuditionGeneratorHandle.BindUFunction(this, TEXT("OnMetaSoundGeneratorHandleCreated"));
	CurrentBuilder->Audition(this, AuditionComponentRef, OnCreateAuditionGeneratorHandle, bReceivesLiveUpdates);

}

void UM2SoundGraphRenderer::SaveMetasoundToAsset()
{

}


void UM2SoundGraphRenderer::SetupFusionNode(FTrackDisplayOptions& TrackRef)
{
	EMetaSoundBuilderResult BuildResult;
	auto FusionNode = CurrentBuilder->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("HarmonixNodes")), FName(TEXT("FusionSamplerStereo"))), BuildResult, 0);
	auto FusionInputs = CurrentBuilder->FindNodeInputs(FusionNode, BuildResult);
	FMetaSoundBuilderNodeInputHandle PatchInput;
	for (const auto& Input : FusionInputs)
	{
		//MSBuilderSystem->Get GetNodeInput(FusionNode, Input, PatchInput, BuildResult);
		FName NodeName;
		FName DataType;
		CurrentBuilder->GetNodeInputData(Input, NodeName, DataType, BuildResult);
		if (NodeName == FName(TEXT("Patch")))
		{
			auto Patch = TrackRef.fusionPatch.Get();
			if (Patch)
			{
				FName PatchInputName = FName(FString::Printf(TEXT("Track_[%d].Patch"), TrackRef.ChannelIndexInParentMidi));
				auto PatchLiteral = MSBuilderSystem->CreateObjectMetaSoundLiteral(Patch);
				auto PatchInputNodeOutput = CurrentBuilder->AddGraphInputNode(PatchInputName, TEXT("FusionPatchAsset"), PatchLiteral, BuildResult);
				//CurrentBuilder->SetNodeInputDefault(Input, PatchLiteral, BuildResult);
				CurrentBuilder->ConnectNodes(PatchInputNodeOutput, Input, BuildResult);
			}
		}
		else if (NodeName == FName(TEXT("MIDI Stream")))
		{
			CurrentBuilder->ConnectNodes(MainMidiStreamOutput, Input, BuildResult);
		}
		else if (NodeName == FName(TEXT("Track Number")))
		{
			FName OutDataType;
			FName TrackInputName = FName(FString::Printf(TEXT("Track_[%d].TrackNum"), TrackRef.ChannelIndexInParentMidi));
			auto ChannelLiteral = MSBuilderSystem->CreateIntMetaSoundLiteral(TrackRef.ChannelIndexInParentMidi, OutDataType);
			auto PatchTrackNodeOutput = CurrentBuilder->AddGraphInputNode(TrackInputName, TEXT("int32"), ChannelLiteral, BuildResult);
			//auto PatchInputNodeOutput = CurrentBuilder->AddGraphInputNode(TEXT("Patch"), TEXT("FusionPatchAsset"), ChannelLiteral, BuildResult);
			//CurrentBuilder->SetNodeInputDefault(Input, ChannelLiteral, BuildResult);
			CurrentBuilder->ConnectNodes(PatchTrackNodeOutput, Input, BuildResult);
		}

	}

	auto FusionOutputs = CurrentBuilder->FindNodeOutputs(FusionNode, BuildResult);
	//auto AudioOuts = GetAvailableOutput();
	bool usedLeft = false;
	for (const auto& Output : FusionOutputs)
	{
		FName NodeName;
		FName DataType;
		CurrentBuilder->GetNodeOutputData(Output, NodeName, DataType, BuildResult);
		auto FreeOutputs = GetFreeAudioOutput();
		if (DataType == FName(TEXT("Audio")))
		{
			CurrentBuilder->ConnectNodes(Output, FreeOutputs[usedLeft ? 0 : 1], BuildResult);
			usedLeft = true;

		}
	}

}

void UM2SoundGraphRenderer::CreateAndRegisterMidiOutput(FTrackDisplayOptions& TrackRef)
{
	// if we render this track in a channel OR output midi, we need to create this track
	bool NeedToCreate = (TrackRef.RenderMode != NoAudio) || TrackRef.CreateMidiOutput;
	//EMetaSoundBuilderResult BuildResult;
	if (NeedToCreate)
	{
		SetupFusionNode(TrackRef);
		MidiOutputNames.Add(FName(TrackRef.trackName));
	}


}



void UM2SoundGraphRenderer::Tick(float DeltaTime)
{
	//UE_LOG(LogTemp, Log, TEXT("Tick! Sequencer Asset Name: %s"), *SessionData->GetName())
	//This should actually only be called in editor, as otherwise the metasound watch output subsystem should handle ticking the watchers
	if (!AuditionComponentRef) return;

		GeneratorHandle->UpdateWatchers();


}

void UM2SoundGraphRenderer::SendSeekCommand(float InSeek)
{
	UE_LOG(LogTemp, Log, TEXT("Seek Command Received! %f"), InSeek)
	if (AuditionComponentRef)
	{
		AuditionComponentRef->SetFloatParameter(FName(TEXT("unDAW.Transport.SeekTarget")), InSeek * 1000.f);
		AuditionComponentRef->SetTriggerParameter(FName(TEXT("unDAW.Transport.Seek")));
	}
}

void UM2SoundGraphRenderer::ReceiveMetaSoundMidiStreamOutput(FName OutputName, const FMetaSoundOutput Value)
{
	//UE_LOG(LogTemp, Log, TEXT("Output Received! %s, Data type: %s"), *OutputName.ToString(), *Value.GetDataTypeName().ToString())
		//auto MidiClockValue = Value.GetDataTypeName();
}

void UM2SoundGraphRenderer::ReceiveMetaSoundMidiClockOutput(FName OutputName, const FMetaSoundOutput Value)
{
	//UE_LOG(LogTemp, Log, TEXT("Output Received! %s, Data type: %s"), *OutputName.ToString(), *Value.GetDataTypeName().ToString())
	Value.Get(CurrentTimestamp);
	OnTimestampUpdated.Broadcast(CurrentTimestamp);
	OnMusicTimestampFromPerformer.ExecuteIfBound(CurrentTimestamp);
	
}


void FindAllMetaSoundClasses()
{
	using namespace Metasound;
	using namespace Metasound::Frontend;

	FMetasoundFrontendClass RegisteredClass;

	auto AllClasses = ISearchEngine::Get().FindAllClasses(true);
	for (const auto MetaClass : AllClasses)
	{
		UE_LOG(LogTemp, Log, TEXT("Class: %s"), *MetaClass.Metadata.GetClassName().GetFullName().ToString())

	}

}




bool SwitchOnBuildResult(EMetaSoundBuilderResult BuildResult)
{
	switch (BuildResult)
	{
	case EMetaSoundBuilderResult::Succeeded:
		return true;
		break;
	case EMetaSoundBuilderResult::Failed:
		return false;
		break;

		default:
			return false;
	}
}

#ifdef WITH_METABUILDERHELPER_TESTS
void UMetasoundBuilderHelperBase::CreateTestWavPlayerBlock()
{
	EMetaSoundBuilderResult BuildResult;
	FSoftObjectPath WavPlayerAssetRef(TEXT("/Script/MetasoundEngine.MetaSoundPatch'/unDAW/BKSystems/MetaSoundBuilderHelperBP_V1/InternalPatches/BK_WavPlayer.BK_WavPlayer'"));
	TScriptInterface<IMetaSoundDocumentInterface> WavPlayerDocument = WavPlayerAssetRef.TryLoad();
	auto WavPlayerNode = CurrentBuilder->AddNodeByClassName(FMetasoundFrontendClassName(TEXT("UE"), TEXT("Wave Player"), TEXT("Stereo")), BuildResult);


	FMetaSoundBuilderNodeInputHandle WavFileInput = CurrentBuilder->FindNodeInputByName(WavPlayerNode, FName(TEXT("Wave Asset")), BuildResult);
	if (SwitchOnBuildResult(BuildResult))
	{
		FSoftObjectPath WavFileAssetRef(TEXT("/Game/onclassical_demo_demicheli_geminiani_pieces_allegro-in-f-major_small-version.onclassical_demo_demicheli_geminiani_pieces_allegro-in-f-major_small-version"));
		auto WavInput = CurrentBuilder->AddGraphInputNode(TEXT("Wave Asset"), TEXT("WaveAsset"), MSBuilderSystem->CreateObjectMetaSoundLiteral(WavFileAssetRef.TryLoad()), BuildResult);
		//CurrentBuilder->SetNodeInputDefault(WavFileInput, MSBuilderSystem->CreateObjectMetaSoundLiteral(WavFileAssetRef.TryLoad()), BuildResult);
		UE_LOG(LogTemp, Log, TEXT("Wav File Input Set! %s"), SwitchOnBuildResult(BuildResult) ? TEXT("yay") : TEXT("nay"))
		FString ResultOut = TEXT("Result: ");
		static const auto AppendToResult = [&ResultOut](const FString& Label, const EMetaSoundBuilderResult& BuildResult) { 
			ResultOut += Label + ":";
			switch (BuildResult)
			{
				case EMetaSoundBuilderResult::Succeeded:
				ResultOut += TEXT("Succeeded\n");
				break;
				case EMetaSoundBuilderResult::Failed:
					ResultOut += TEXT("Failed\n");
					break;


			default:
				ResultOut += TEXT("Unknown");
				break;
			}
				};
		if (SwitchOnBuildResult(BuildResult))
		{
			CurrentBuilder->ConnectNodes(WavInput, WavFileInput, BuildResult);
			AppendToResult(TEXT("Wave Input"), BuildResult);
			auto PlayPin = CurrentBuilder->FindNodeInputByName(WavPlayerNode, FName(TEXT("Play")), BuildResult);
			AppendToResult(TEXT("Find Play Pin"), BuildResult);
			auto AudioLeft = CurrentBuilder->FindNodeOutputByName(WavPlayerNode, FName(TEXT("Out Left")), BuildResult);
			AppendToResult(TEXT("Find Out Left"), BuildResult);
			auto AudioRight = CurrentBuilder->FindNodeOutputByName(WavPlayerNode, FName(TEXT("Out Right")), BuildResult);
			AppendToResult(TEXT("Find Out Right"), BuildResult);
			CurrentBuilder->ConnectNodes(OnPlayOutputNode, PlayPin, BuildResult);
			AppendToResult(TEXT("On Play"), BuildResult);

			auto WavePlayerAudioOuts = CurrentBuilder->FindNodeOutputsByDataType(WavPlayerNode, BuildResult, FName(TEXT("Audio")));


			for (int i = 0; i < AudioOuts.Num(); i++)
			{
				if (WavePlayerAudioOuts.IsValidIndex(i))
				{
					CurrentBuilder->ConnectNodes( WavePlayerAudioOuts[i], AudioOuts[i], BuildResult);
					AppendToResult(FString::Printf(TEXT("Connect Node Output %d"), i), BuildResult);
				}
				else
				{
					UE_LOG(LogTemp, Log, TEXT("Wave Player Audio Out Index %d Not Valid!"), i)
						break;
				}
			}

		}

		UE_LOG(LogTemp, Log, TEXT("Build Results: \n %s"), *ResultOut)

		//CurrentBuilder->SetNodeInputDefault(WavFileInput, MSBuilderSystem->CreateObjectMetaSoundLiteral(WavFileAssetRef.TryLoad()), BuildResult);

	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Wav File Input Not Set! %s"), SwitchOnBuildResult(BuildResult) ? TEXT("yay") : TEXT("nay"))
	}	
}

#endif //WITH_METABUILDERHELPER_TESTS

void UM2SoundGraphRenderer::CreateInputsFromMidiTracks()
{
	// Create the inputs from the midi tracks
	//for (auto& [trackID, trackOptions] : *MidiTracks)
	//{
	//	CreateAndRegisterMidiOutput(trackOptions);
	//	// Create the input node
	//	//FMetaSoundBuilderNodeInputHandle InputNode = CurrentBuilder->CreateInputNode(FName(*trackOptions.trackName), trackOptions.trackColor, trackOptions.ChannelIndexInParentMidi)
	//}



}

void UM2SoundGraphRenderer::CreateMixerPatchBlock()
{

	using namespace Metasound::Frontend;
	UMetaSoundPatch* Patch = NewObject<UMetaSoundPatch>(this);
	check(Patch);

	auto Builder = FMetaSoundFrontendDocumentBuilder(Patch);
	Builder.InitDocument();

	// create the mixer patch
	EMetaSoundBuilderResult BuildResult;
	const auto PatchBuilder = MSBuilderSystem->CreatePatchBuilder(FName(TEXT("Mixer Patch")), BuildResult);
	//const auto PB2 = MSBuilderSystem->Trabsuebt
	PatchBuilder->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
		, BuildResult);

	if (SwitchOnBuildResult(BuildResult))
	{
		//PatchBuilder->
		FMetaSoundBuilderOptions PatchOptions;

		PatchOptions.bAddToRegistry = false;


		FSoftObjectPath MixerPatchAssetRef(TEXT("/Script/MetasoundEngine.MetaSoundPatch'/Game/MixerPatch.MixerPatch'"));

		auto MixerPatch = PatchBuilder->Build(Patch, PatchOptions);

		CurrentBuilder->AddNode(MixerPatch, BuildResult);
	}
}

	void UM2SoundGraphRenderer::CreateMixerNodesSpaghettiBlock()
	{
		// create master mixer
		EMetaSoundBuilderResult BuildResult;
		const auto MasterMixerNode = CurrentBuilder->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
			, BuildResult);

		auto MixerOutputs = CurrentBuilder->FindNodeOutputs(MasterMixerNode, BuildResult);
		//auto AudioOuts = GetAvailableOutput();
		bool usedLeft = false;
		for (const auto& Output : MixerOutputs)
		{
			FName NodeName;
			FName DataType;
			CurrentBuilder->GetNodeOutputData(Output, NodeName, DataType, BuildResult);
			if (DataType == FName(TEXT("Audio")))
			{
				CurrentBuilder->ConnectNodes(Output, AudioOuts[usedLeft ? 1 : 0], BuildResult);
				usedLeft = true;

			}
		}

		MasterOutputsArray.Append(CurrentBuilder->FindNodeInputs(MasterMixerNode, BuildResult));
		
	}



	void UM2SoundGraphRenderer::AttachAnotherMasterMixerToOutput()
	{
		EMetaSoundBuilderResult BuildResult;
		const auto MasterMixerNode = CurrentBuilder->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
			, BuildResult);
		
				auto MixerOutputs = CurrentBuilder->FindNodeOutputs(MasterMixerNode, BuildResult);
				//auto AudioOuts = GetAvailableOutput();
				CurrentBuilder->ConnectNodes(MixerOutputs[1], MasterOutputsArray.Pop(), BuildResult);
				CurrentBuilder->ConnectNodes(MixerOutputs[0], MasterOutputsArray.Pop(), BuildResult);
				MasterOutputsArray.Append(CurrentBuilder->FindNodeInputs(MasterMixerNode, BuildResult));
	}

	TArray<FMetaSoundBuilderNodeInputHandle> UM2SoundGraphRenderer::GetFreeAudioOutput()
	{
		TArray<FMetaSoundBuilderNodeInputHandle> FreeOutputs;
		
		if (MasterOutputsArray.Num() > 2)
		{
			FreeOutputs.Add(MasterOutputsArray.Pop());
			FreeOutputs.Add(MasterOutputsArray.Pop());
			return FreeOutputs;
		}
		else
		{
			AttachAnotherMasterMixerToOutput();
			return GetFreeAudioOutput();
		}
	}


	// we do not use this due to the issues with the frontend only registering the patch when the metasound graph is opened, which is annoying.
	bool UM2SoundGraphRenderer::CreateMidiPlayerBlock()
{
		// Create the midi player block
	EMetaSoundBuilderResult BuildResult;
	FSoftObjectPath MidiPlayerAssetRef(TEXT("/unDAW/Patches/System/unDAW_MainClock.unDAW_MainClock"));
	TScriptInterface<IMetaSoundDocumentInterface> MidiPlayerDocument = MidiPlayerAssetRef.TryLoad();
	UMetaSoundPatch* MidiPatch = Cast<UMetaSoundPatch>(MidiPlayerDocument.GetObject());
	//MidiPatch->RegisterGraphWithFrontend();
	MidiPlayerNode = CurrentBuilder->AddNode(MidiPlayerDocument, BuildResult);

	if (SwitchOnBuildResult(BuildResult))
	{
		CurrentBuilder->ConnectNodeInputsToMatchingGraphInterfaceInputs(MidiPlayerNode, BuildResult);
	}

	if (SwitchOnBuildResult(BuildResult))
	{
		CurrentBuilder->ConnectNodeOutputsToMatchingGraphInterfaceOutputs(MidiPlayerNode, BuildResult);
	}

	FMetaSoundBuilderNodeInputHandle MidiFileInput = CurrentBuilder->FindNodeInputByName(MidiPlayerNode, FName(TEXT("MIDI File")), BuildResult);
	MainMidiStreamOutput = CurrentBuilder->FindNodeOutputByName(MidiPlayerNode, FName(TEXT("unDAW.Midi Stream")), BuildResult);
	auto MidiInputPinOutputHandle = CurrentBuilder->AddGraphInputNode(TEXT("Midi File"), TEXT("MidiAsset"), MSBuilderSystem->CreateObjectMetaSoundLiteral(SessionData->HarmonixMidiFile), BuildResult);
	CurrentBuilder->ConnectNodes(MidiInputPinOutputHandle, MidiFileInput, BuildResult);


	return SwitchOnBuildResult(BuildResult);

	//FMetaSoundBuilderNodeInputHandle MidiPlayerNode = CurrentBuilder->CreateMidiPlayerNode(FName(TEXT("Midi Player")), FLinearColor::Green, 0);
}

void UM2SoundGraphRenderer::CreateAndAuditionPreviewAudioComponent()
{
	PlayState = EBKPlayState::ReadyToPlay;
	FOnCreateAuditionGeneratorHandleDelegate OnCreateAuditionGeneratorHandle;
	OnCreateAuditionGeneratorHandle.BindUFunction(this, TEXT("OnMetaSoundGeneratorHandleCreated"));
	CurrentBuilder->Audition(this, AuditionComponentRef, OnCreateAuditionGeneratorHandle, true);
}

void UM2SoundGraphRenderer::OnMetaSoundGeneratorHandleCreated(UMetasoundGeneratorHandle* Handle)
{

	UMetaSoundAssetSubsystem* AssetSubsystem = GEngine->GetEngineSubsystem<UMetaSoundAssetSubsystem>();
	//FMetasoundAssetBase test = Cast<UMetaSoundSource>();
	AssetSubsystem->Get()->AddOrUpdateAsset(*AuditionComponentRef->GetSound()->_getUObject());
	//UE_LOG(LogTemp, Log, TEXT("Handle Created!"))
	GeneratorHandle = Handle;
	AuditionComponentRef->GetSound()->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
	AuditionComponentRef->SetTriggerParameter(FName("unDAW.Transport.Prepare"));
	OnMidiClockOutputReceived.BindLambda([this](FName OutputName, const FMetaSoundOutput Value) { ReceiveMetaSoundMidiClockOutput(OutputName, Value); });
	OnMidiStreamOutputReceived.BindLambda([this](FName OutputName, const FMetaSoundOutput Value) { ReceiveMetaSoundMidiStreamOutput(OutputName, Value); });


	bool CanWatchStream = GeneratorHandle->WatchOutput(FName("unDAW.Midi Stream"), OnMidiStreamOutputReceived);
	bool CanWatchClock = GeneratorHandle->WatchOutput(FName("unDAW.Midi Clock"), OnMidiClockOutputReceived);
	bShouldTick = true;

	OnDAWPerformerReady.Broadcast();
}




