// Fill out your copyright notice in the Description page of Project Settings.


#include "UnDAWSequencePerformer.h"
#include "Engine/Engine.h"
#include "IAudioParameterInterfaceRegistry.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "MetasoundAssetSubsystem.h"
#include "MetasoundAssetBase.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"


void UDAWSequencerPerformer::SendTransportCommand(EBKTransportCommands Command)
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

void UDAWSequencerPerformer::InitBuilderHelper(FString BuilderName, UAudioComponent* InAuditionComponent)
{
	MSBuilderSystem = GEngine->GetEngineSubsystem<UMetaSoundBuilderSubsystem>();
	

	if (AuditionComponentRef)
	{
		AuditionComponentRef->Stop();
		AuditionComponentRef->DestroyComponent();
	}



	AuditionComponentRef = InAuditionComponent;


	//ParentWorld = World;
	//FMetaSoundBuilderNodeOutputHandle OnPlayOutputNode;
	FMetaSoundBuilderNodeInputHandle OnFinished;


	
	EMetaSoundBuilderResult BuildResult;
	//OutputFormat = SourceOutputFormat;
	//SessionData->MasterOptions.OutputFormat
	CurrentBuilder = MSBuilderSystem->CreateSourceBuilder(FName(BuilderName), OnPlayOutputNode, OnFinished, AudioOuts, BuildResult, SessionData->MasterOptions.OutputFormat, false);
	MSBuilderSystem->RegisterSourceBuilder(FName(BuilderName), CurrentBuilder);
	CurrentBuilder->AddInterface(FName(TEXT("unDAW Session Renderer")), BuildResult);
	CreateMixerNodesSpaghettiBlock();
	//CreateMidiPlayerBlock();
	GenerateMidiPlayerAndTransport();
	CreateInputsFromMidiTracks();

#ifdef WITH_METABUILDERHELPER_TESTS
	CreateTestWavPlayerBlock();
#endif //WITH_TESTS

	PerformBpInitialization();

}

TArray<UMetaSoundSource*> UDAWSequencerPerformer::GetAllMetasoundSourcesWithInstrumentInterface()
{
	auto AllObjectsArray = TArray<UMetaSoundSource*>();
	auto OnlyImplementingArray = TArray<UMetaSoundSource*>();
	

	GetObjectsOfClass<UMetaSoundSource>(AllObjectsArray);
	for (const auto& source : AllObjectsArray)
	{
		if (source->ImplementsParameterInterface(unDAW::Metasounds::FunDAWInstrumentRendererInterface::GetInterface())) OnlyImplementingArray.Add(source);
	}
	
	return OnlyImplementingArray;
}

TArray<UMetaSoundPatch*> UDAWSequencerPerformer::GetAllMetasoundPatchesWithInstrumentInterface()
{
	auto AllObjectsArray = TArray<UMetaSoundPatch*>();
	auto OnlyImplementingArray = TArray<UMetaSoundPatch*>();
	GetObjectsOfClass<UMetaSoundPatch>(AllObjectsArray);
	auto interface = unDAW::Metasounds::FunDAWInstrumentRendererInterface::GetInterface();

	const FMetasoundFrontendVersion Version{ interface->GetName(), { interface->GetVersion().Major, interface->GetVersion().Minor } };

	for (const auto& patch : AllObjectsArray)
	{
		if (patch->GetDocumentChecked().Interfaces.Contains(Version)) OnlyImplementingArray.Add(patch);
	}
	return OnlyImplementingArray;
}

TArray<UMetaSoundPatch*> UDAWSequencerPerformer::GetAllMetasoundPatchesWithInsertInterface()
{
	auto AllObjectsArray = TArray<UMetaSoundPatch*>();
	auto OnlyImplementingArray = TArray<UMetaSoundPatch*>();
	GetObjectsOfClass<UMetaSoundPatch>(AllObjectsArray);
	auto interface = unDAW::Metasounds::FunDAWCustomInsertInterface::GetInterface();

	const FMetasoundFrontendVersion Version{ interface->GetName(), { interface->GetVersion().Major, interface->GetVersion().Minor } };

	for (const auto& patch : AllObjectsArray)
	{
		if (patch->GetDocumentChecked().Interfaces.Contains(Version)) OnlyImplementingArray.Add(patch);
	}
	return OnlyImplementingArray;
}


void UDAWSequencerPerformer::SetupFusionNode(FTrackDisplayOptions& TrackRef)
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

void UDAWSequencerPerformer::CreateAndRegisterMidiOutput(FTrackDisplayOptions& TrackRef)
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

void UDAWSequencerPerformer::CreateFusionPlayerForMidiTrack()
{
}

void UDAWSequencerPerformer::ConnectTransportPinsToInterface(FMetaSoundNodeHandle& TransportNode)
{

	EMetaSoundBuilderResult BuildResult;
	TArray<FMetaSoundBuilderNodeInputHandle> TriggerToTransportInputs = CurrentBuilder->FindNodeInputs(TriggerToTransportNode, BuildResult);

	for(const auto& Input : TriggerToTransportInputs)
	{
		FName NodeName;
		FName DataType;
		CurrentBuilder->GetNodeInputData(Input, NodeName, DataType, BuildResult);

		if (NodeName == FName(TEXT("Prepare")))
		{
			CurrentBuilder->ConnectNodes(OnPlayOutputNode, Input, BuildResult);
		}

		if (NodeName == FName(TEXT("Play")))
		{
			CurrentBuilder->ConnectNodeInputToGraphInput(FName(TEXT("unDAW.Transport.Play")), Input, BuildResult);
		}

		if (NodeName == FName(TEXT("Stop")))
		{
			CurrentBuilder->ConnectNodeInputToGraphInput(FName(TEXT("unDAW.Transport.Stop")), Input, BuildResult);
		}

		if (NodeName == FName(TEXT("Pause")))
		{
			CurrentBuilder->ConnectNodeInputToGraphInput(FName(TEXT("unDAW.Transport.Pause")), Input, BuildResult);
		}
		if (NodeName == FName(TEXT("Trigger Seek")))
		{
			CurrentBuilder->ConnectNodeInputToGraphInput(FName(TEXT("unDAW.Transport.Seek")), Input, BuildResult);
		}

		if (NodeName == FName(TEXT("Seek Target")))
		{
			auto FloatToTimeCastNode = CurrentBuilder->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("HarmonixNodes")), FName(TEXT("TimeMsToSeekTarget"))), BuildResult, 0);
			auto FloatToTimeInput = CurrentBuilder->FindNodeInputByName(FloatToTimeCastNode, FName(TEXT("Time (ms)")), BuildResult);
			auto FloatToTimeOutput = CurrentBuilder->FindNodeOutputByName(FloatToTimeCastNode, FName(TEXT("Seek Target")), BuildResult);
			CurrentBuilder->ConnectNodeInputToGraphInput(FName(TEXT("unDAW.Transport.SeekTarget")), FloatToTimeInput, BuildResult);
			CurrentBuilder->ConnectNodes(FloatToTimeOutput, Input, BuildResult);
		}

	}
}

void UDAWSequencerPerformer::GenerateMidiPlayerAndTransport()
{
	EMetaSoundBuilderResult BuildResult;

	TriggerToTransportNode = CurrentBuilder->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("HarmonixNodes")), FName(TEXT("TriggerToTransport"))), BuildResult, 0);

	ConnectTransportPinsToInterface(TriggerToTransportNode);

	MidiPlayerNode = CurrentBuilder->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("HarmonixNodes")), FName(TEXT("MIDIPlayer"))), BuildResult, 0);
	FMetaSoundBuilderNodeInputHandle MidiFileInput = CurrentBuilder->FindNodeInputByName(MidiPlayerNode, FName(TEXT("MIDI File")), BuildResult);
	MainMidiStreamOutput = CurrentBuilder->FindNodeOutputByName(MidiPlayerNode, FName(TEXT("MIDI File")), BuildResult);
	auto MidiInputPinOutputHandle = CurrentBuilder->AddGraphInputNode(TEXT("Midi File"), TEXT("MidiAsset"), MSBuilderSystem->CreateObjectMetaSoundLiteral(SessionData->HarmonixMidiFile), BuildResult);
	CurrentBuilder->ConnectNodes(MidiInputPinOutputHandle, MidiFileInput, BuildResult);

	//connect midi player 'Transport' to 'Transport Node' transport pin
	auto MidiPlayerTransportPin = CurrentBuilder->FindNodeInputByName(MidiPlayerNode, FName(TEXT("Transport")), BuildResult);
	auto TransportNodeTransportOutPin = CurrentBuilder->FindNodeOutputByName(TriggerToTransportNode, FName(TEXT("Transport")), BuildResult);
	CurrentBuilder->ConnectNodes(TransportNodeTransportOutPin, MidiPlayerTransportPin, BuildResult);
	MainMidiStreamOutput = CurrentBuilder->FindNodeOutputByName(MidiPlayerNode, FName(TEXT("MIDI Stream")), BuildResult);

}

void UDAWSequencerPerformer::CreateCustomPatchPlayerForMidiTrack()
{
}

void UDAWSequencerPerformer::ChangeFusionPatchInTrack(int TrackIndex, UFusionPatch* NewPatch)
{
	UE_LOG(LogTemp, Log, TEXT("Change Fusion Patch for Track %d"), TrackIndex)
		if (AuditionComponentRef && MidiTracks->Contains(TrackIndex))
		{
		auto ParamName = FName(FString::Printf(TEXT("Track_[%d].Patch"), MidiTracks->Find(TrackIndex)->ChannelIndexInParentMidi));
		AuditionComponentRef->SetObjectParameter(ParamName, NewPatch);
		}
}

void UDAWSequencerPerformer::SendSeekCommand(float InSeek)
{
	UE_LOG(LogTemp, Log, TEXT("Seek Command Received! %f"), InSeek)
	if (AuditionComponentRef)
	{
		AuditionComponentRef->SetFloatParameter(FName(TEXT("unDAW.Transport.SeekTarget")), InSeek * 1000.f);
		AuditionComponentRef->SetTriggerParameter(FName(TEXT("unDAW.Transport.Seek")));
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

void UDAWSequencerPerformer::CreateInputsFromMidiTracks()
{
	// Create the inputs from the midi tracks
	for (auto& [trackID, trackOptions] : *MidiTracks)
	{
		CreateAndRegisterMidiOutput(trackOptions);
		// Create the input node
		//FMetaSoundBuilderNodeInputHandle InputNode = CurrentBuilder->CreateInputNode(FName(*trackOptions.trackName), trackOptions.trackColor, trackOptions.ChannelIndexInParentMidi)
	}



}

void UDAWSequencerPerformer::CreateMixerPatchBlock()
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

	void UDAWSequencerPerformer::CreateMixerNodesSpaghettiBlock()
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



	void UDAWSequencerPerformer::AttachAnotherMasterMixerToOutput()
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

	TArray<FMetaSoundBuilderNodeInputHandle> UDAWSequencerPerformer::GetFreeAudioOutput()
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
	bool UDAWSequencerPerformer::CreateMidiPlayerBlock()
{
		// Create the midi player block
	EMetaSoundBuilderResult BuildResult;
	FSoftObjectPath MidiPlayerAssetRef(TEXT("/unDAW/BKSystems/MetaSoundBuilderHelperBP_V1/InternalPatches/BK_MetaClockManager.BK_MetaClockManager"));
	TScriptInterface<IMetaSoundDocumentInterface> MidiPlayerDocument = MidiPlayerAssetRef.TryLoad();
	UMetaSoundPatch* MidiPatch = Cast<UMetaSoundPatch>(MidiPlayerDocument.GetObject());
	MidiPatch->RegisterGraphWithFrontend();
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
	MainMidiStreamOutput = CurrentBuilder->FindNodeOutputByName(MidiPlayerNode, FName(TEXT("MIDI File")), BuildResult);
	auto MidiInputPinOutputHandle = CurrentBuilder->AddGraphInputNode(TEXT("Midi File"), TEXT("MidiAsset"), MSBuilderSystem->CreateObjectMetaSoundLiteral(SessionData->HarmonixMidiFile), BuildResult);
	CurrentBuilder->ConnectNodes(MidiInputPinOutputHandle, MidiFileInput, BuildResult);


	return SwitchOnBuildResult(BuildResult);

	//FMetaSoundBuilderNodeInputHandle MidiPlayerNode = CurrentBuilder->CreateMidiPlayerNode(FName(TEXT("Midi Player")), FLinearColor::Green, 0);
}

void UDAWSequencerPerformer::CreateAndAuditionPreviewAudioComponent()
{
	PlayState = EBKPlayState::ReadyToPlay;
	FOnCreateAuditionGeneratorHandleDelegate OnCreateAuditionGeneratorHandle;
	OnCreateAuditionGeneratorHandle.BindUFunction(this, TEXT("OnMetaSoundGeneratorHandleCreated"));
	CurrentBuilder->Audition(this, AuditionComponentRef, OnCreateAuditionGeneratorHandle, true);
}

void UDAWSequencerPerformer::OnMetaSoundGeneratorHandleCreated(UMetasoundGeneratorHandle* Handle)
{
	
	UMetaSoundAssetSubsystem* AssetSubsystem = GEngine->GetEngineSubsystem<UMetaSoundAssetSubsystem>();
	//FMetasoundAssetBase test = Cast<UMetaSoundSource>();
	AssetSubsystem->Get()->AddOrUpdateAsset(*AuditionComponentRef->GetSound()->_getUObject());
	UE_LOG(LogTemp, Log, TEXT("Handle Created!"))
	GeneratorHandle = Handle;
	AuditionComponentRef->GetSound()->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
	AuditionComponentRef->SetTriggerParameter(FName("unDAW.Transport.Prepare"));
	//AuditionComponentRef->SetTriggerParameter(FName("unDAW.Transport.Play"));
	OnDAWPerformerReady.Broadcast();
}




