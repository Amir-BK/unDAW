// Fill out your copyright notice in the Description page of Project Settings.

#include "M2SoundGraphData.h"
#include "M2SoundGraphRenderer.h"
#include "M2SoundGraphStatics.h"
#include "Metasound.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"
#include "Vertexes/M2SoundVertex.h"
#include <EditableMidiFile.h>

DEFINE_LOG_CATEGORY(unDAWDataLogs);

struct FEventsWithIndex
{
	FMidiEvent event;
	int32 eventIndex;
};

void UDAWSequencerData::RebuildVertex(UM2SoundVertex* Vertex)
{
	Vertex->BuildVertex();
	Vertex->CollectParamsForAutoConnect();
	Vertex->UpdateConnections();
}

void UDAWSequencerData::UpdateVertexConnections(UM2SoundVertex* Vertex)
{
	Vertex->UpdateConnections();

}

void UDAWSequencerData::ReceiveAudioParameter(FAudioParameter Parameter)
{
	if (AuditionComponent) AuditionComponent->SetParameter(MoveTemp(Parameter));
}

void UDAWSequencerData::Tick(float DeltaTime)
{
	if (!AuditionComponent) return;

	GeneratorHandle->UpdateWatchers();

}

bool UDAWSequencerData::IsTickable() const
{
	return bShouldTick;
}

void UDAWSequencerData::ReceiveMetaSoundMidiStreamOutput(FName OutputName, const FMetaSoundOutput Value)
{
}

void UDAWSequencerData::ReceiveMetaSoundMidiClockOutput(FName OutputName, const FMetaSoundOutput Value)
{
	Value.Get(CurrentTimestampData);

}

void UDAWSequencerData::OnMetaSoundGeneratorHandleCreated(UMetasoundGeneratorHandle* Handle)
{
	GeneratorHandle = Handle;
	PlayState = ReadyToPlay;

	OnMidiClockOutputReceived.BindLambda([this](FName OutputName, const FMetaSoundOutput Value) { ReceiveMetaSoundMidiClockOutput(OutputName, Value); });
	OnMidiStreamOutputReceived.BindLambda([this](FName OutputName, const FMetaSoundOutput Value) { ReceiveMetaSoundMidiStreamOutput(OutputName, Value); });

	bool CanWatchStream = GeneratorHandle->WatchOutput(FName("unDAW.Midi Stream"), OnMidiStreamOutputReceived);
	bool CanWatchClock = GeneratorHandle->WatchOutput(FName("unDAW.Midi Clock"), OnMidiClockOutputReceived);
	bShouldTick = true;
}

void UDAWSequencerData::SendTransportCommand(EBKTransportCommands Command)
{
	if (AuditionComponent != nullptr)
	{
		switch (Command)
		{

		case EBKTransportCommands::Play:
			AuditionComponent->SetTriggerParameter(FName(TEXT("unDAW.Transport.Play")));
			PlayState = EBKPlayState::Playing;
			break;

		case EBKTransportCommands::Stop:
			AuditionComponent->SetTriggerParameter(FName(TEXT("unDAW.Transport.Stop")));
			PlayState = EBKPlayState::ReadyToPlay;

			break;

		case EBKTransportCommands::Pause:
			AuditionComponent->SetTriggerParameter(FName(TEXT("unDAW.Transport.Pause")));
			PlayState = EBKPlayState::Paused;
			break;
		default:
			break;


		}
	}
}

void UDAWSequencerData::SendSeekCommand(float InSeek)
{
	UE_LOG(LogTemp, Log, TEXT("Seek Command Received! %f"), InSeek)
		if (AuditionComponent)
		{
			AuditionComponent->SetFloatParameter(FName(TEXT("unDAW.Transport.SeekTarget")), InSeek * 1000.f);
			AuditionComponent->SetTriggerParameter(FName(TEXT("unDAW.Transport.Seek")));
		}
}

void UDAWSequencerData::AddVertex(UM2SoundVertex* Vertex)
{
	Vertexes.Add(Vertex);

	//update builder, in the future we may want to be a little bit more conservative with this
	//MSBuilderSystem->RegisterBuilder(BuilderName, BuilderContext);
	
	OnVertexAdded.Broadcast(Vertex);
}

inline void UDAWSequencerData::InitVertexesFromFoundMidiTracks(TArray<TTuple<int, int>> InTracks) {
	auto PianoPatchPath = FSoftObjectPath(TEXT("/Harmonix/Examples/Patches/Piano.Piano"));
	Vertexes.Empty();

	UFusionPatch* PianoPatch = static_cast<UFusionPatch*>(PianoPatchPath.TryLoad());

	M2TrackMetadata.Empty();
	for (const auto& [trackID, channelID] : InTracks)
	{
		FTrackDisplayOptions newTrack;
		bool bIsPrimaryChannel = HarmonixMidiFile->GetTrack(trackID)->GetPrimaryMidiChannel() == channelID;
		newTrack.ChannelIndexInParentMidi = bIsPrimaryChannel ? 0 : channelID;
		newTrack.TrackIndexInParentMidi = trackID;

		newTrack.trackName = *HarmonixMidiFile->GetTrack(trackID)->GetName() + " Ch: " + FString::FromInt(channelID) + " Tr: " + FString::FromInt(trackID);
		newTrack.fusionPatch = PianoPatch;
		int IndexOfNewTrack = M2TrackMetadata.Add(newTrack);

		FLinearColor trackColor;

		switch (IndexOfNewTrack)
		{
		case 0:
			trackColor = FLinearColor::Red;
			break;
		case 1:
			trackColor = FLinearColor::White;
			break;
		case 2:
			trackColor = FLinearColor::Green;
			break;

		default:
			trackColor = FLinearColor::MakeRandomSeededColor(channelID * 16 + trackID);
			break;
		}
		M2TrackMetadata[IndexOfNewTrack].trackColor = trackColor;


		//AddVertex(NewInput);
		//TrackInputs.Add(IndexOfNewTrack, NewInput);
		UM2SoundGraphStatics::CreateDefaultVertexesFromInputData(this, IndexOfNewTrack);
	}
}

FTrackDisplayOptions& UDAWSequencerData::GetTracksDisplayOptions(const int& ID)
{
	if (M2TrackMetadata.IsValidIndex(ID))
	{
		return M2TrackMetadata[ID];
	}
	else
	{
		return InvalidTrackRef;
	}
}

void UDAWSequencerData::CalculateSequenceDuration()
{
	if (HarmonixMidiFile)
	{
		SequenceDuration = HarmonixMidiFile->GetSongMaps()->GetSongLengthMs();
	}
}

void UDAWSequencerData::AddLinkedMidiEvent(FLinkedMidiEvents PendingNote)
{
	auto MidiFileCopy = NewObject<UEditableMidiFile>(this);
	MidiFileCopy->LoadFromHarmonixBaseFile(HarmonixMidiFile);
	
	//auto NewTrack = MidiFileCopy->AddTrack(FString::Printf(TEXT("Track %d"), TrackIndex++));
	auto TrackMetaData = GetTracksDisplayOptions(PendingNote.TrackId);

	UE_LOG(unDAWDataLogs, Verbose, TEXT("Pushing note to track %d, channel %d"), TrackMetaData.TrackIndexInParentMidi, TrackMetaData.ChannelIndexInParentMidi)

		auto TargetTrack = MidiFileCopy->GetTrack(TrackMetaData.TrackIndexInParentMidi);
	auto StartMessage = FMidiMsg::CreateNoteOn(TrackMetaData.ChannelIndexInParentMidi, PendingNote.pitch, 127);
	auto EndMessage = FMidiMsg::CreateNoteOff(TrackMetaData.ChannelIndexInParentMidi, PendingNote.pitch);
	auto NewStartNoteMidiEvent = FMidiEvent(PendingNote.StartTick, StartMessage);
	auto NewEndNoteMidiEvent = FMidiEvent(PendingNote.EndTick, EndMessage);

	TargetTrack->AddEvent(NewStartNoteMidiEvent);
	TargetTrack->AddEvent(NewEndNoteMidiEvent);

	PendingLinkedMidiNotesMap.Add(PendingNote);
	MidiFileCopy->SortAllTracks();

	HarmonixMidiFile = MidiFileCopy;

	if(AuditionComponent) AuditionComponent->SetObjectParameter(FName(TEXT("Midi File")), HarmonixMidiFile);
}

//TODO: I don't like this implementation, the linked notes should be created by demand from the midifile and only stored transiently
void UDAWSequencerData::PopulateFromMidiFile(UMidiFile* inMidiFile)
{
	
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Populating Sequencer Data from Midi File"))
		
	
	TArray<TTuple<int, int>> FoundChannels;
	LinkedNoteDataMap.Empty();
	PendingLinkedMidiNotesMap.Empty();

	//Outputs.Empty();
	//TrackInputs.Empty();
	//Patches.Empty();
	if (!IsRecreatingMidiFile)
	{
		if (AuditionComponent)
		{
			AuditionComponent->Stop();
			//AuditionComponent->DestroyComponent();
		}
		
		Vertexes.Empty();
		FString BuilderString = FString::Printf(TEXT("unDAW-%s"), *GetName());
		BuilderName = FName(BuilderString);
	}

	HarmonixMidiFile = inMidiFile;


	//MidiSongMap = HarmonixMidiFile->GetSongMaps();

	int numTracks = 0;
	int numTracksRaw = 0;

	for (auto& track : HarmonixMidiFile->GetTracks())
	{
		//if track has no events we can continue, but this never happens, it might not have note events but it has events.
		int numTracksInternal = numTracksRaw++;
		if (track.GetEvents().IsEmpty()) continue;

		int TrackMainChannel = track.GetPrimaryMidiChannel();

		TArray<FLinkedMidiEvents*> linkedNotes;
		TMap<int32, FEventsWithIndex> unlinkedNotesIndexed;

		//track.GetEvent(32)

		// sort events, right now only notes
		for (int32 index = 0; const auto & MidiEvent : track.GetEvents())
		{
			switch (MidiEvent.GetMsg().Type) {
			case FMidiMsg::EType::Std:
				if (MidiEvent.GetMsg().IsNoteOn())
				{
					//unlinkedNotes.Add(MidiEvent.GetMsg().GetStdData1(), MidiEvent);
					unlinkedNotesIndexed.Add(MidiEvent.GetMsg().GetStdData1(), FEventsWithIndex{ MidiEvent, index });
				};

				if (MidiEvent.GetMsg().IsNoteOff())
				{
					if (unlinkedNotesIndexed.Contains(MidiEvent.GetMsg().GetStdData1()))
					{
						const int midiChannel = MidiEvent.GetMsg().GetStdChannel();

						if (midiChannel == unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].event.GetMsg().GetStdChannel())
						{
							FLinkedMidiEvents foundPair = FLinkedMidiEvents(unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].event, MidiEvent,
								unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].eventIndex, index);

							UE_LOG(unDAWDataLogs, Verbose, TEXT("Found note pair, track %d, channel %d, midi main channel %d"), numTracksInternal, midiChannel, TrackMainChannel)
								foundPair.TrackId = FoundChannels.AddUnique(TTuple<int, int>(numTracksInternal, midiChannel));
							foundPair.ChannelId = midiChannel;
							foundPair.CalculateDuration(HarmonixMidiFile->GetSongMaps());
							linkedNotes.Add(&foundPair);
							// sort the tracks into channels
							if (LinkedNoteDataMap.Contains(midiChannel))
							{
								LinkedNoteDataMap[midiChannel].LinkedNotes.Add(foundPair);
							}
							else {
								LinkedNoteDataMap.Add(TTuple<int, TArray<FLinkedMidiEvents>>(midiChannel, TArray<FLinkedMidiEvents>()));
								LinkedNoteDataMap[midiChannel].LinkedNotes.Add(foundPair);
							}
						}
					}
				};

				break;
			case FMidiMsg::EType::Tempo:
				//UE_LOG(unDAWDataLogs, Verbose, TEXT("We receive a tempo event! data1 %d data2 %d"), MidiEvent.GetMsg().Data1, MidiEvent.GetMsg().Data2)
					//MidiEvent.GetMsg().Data1
				TempoEvents.Add(MidiEvent);
				break;
			case FMidiMsg::EType::TimeSig:
				//UE_LOG(unDAWDataLogs, Verbose, TEXT("We receive a time signature event!"))
				TimeSignatureEvents.Add(MidiEvent);
				break;
			case FMidiMsg::EType::Text:
				//UE_LOG(unDAWDataLogs, Verbose, TEXT("We receive a text event??? %s"), *MidiEvent.GetMsg().ToString(MidiEvent.GetMsg()))
				break;
			case FMidiMsg::EType::Runtime:
				//UE_LOG(unDAWDataLogs, Verbose, TEXT("We receive a runtime event???"))
				break;
			}

			++index;
		}
		// if we couldn't find any linked notes this track is a control track, contains no notes.

		if (LinkedNoteDataMap.IsEmpty()) continue;

		//CreateBuilderHelper();
	}

	CalculateSequenceDuration();
	//if we're recreating the midi file we don't need to do anything else
	if (IsRecreatingMidiFile) 
	{
		IsRecreatingMidiFile = false;
		
	}
	else {
		FindOrCreateBuilderForAsset(true);
		InitVertexesFromFoundMidiTracks(FoundChannels);
	}


#if WITH_EDITOR
	if (M2SoundGraph)
	{
		M2SoundGraph->InitializeGraph();
	}
#endif

	
}

void UDAWSequencerData::FindOrCreateBuilderForAsset(bool bResetBuilder)
{
	MSBuilderSystem = GEngine->GetEngineSubsystem<UMetaSoundBuilderSubsystem>();

	if(!bVertexParamUpdatesAreBound)
	{
		OnAudioParameterFromVertex.AddDynamic(this, &UDAWSequencerData::ReceiveAudioParameter);
		bVertexParamUpdatesAreBound = true;
	}

	EMetaSoundBuilderResult BuildResult;

	auto SavedBuilder = MSBuilderSystem->FindBuilder(BuilderName);
	//MSBuilderSystem->
	//log SavedBuilder result

	if(SavedBuilder == nullptr) UE_LOG(unDAWDataLogs, Verbose, TEXT("Builder %s not found"), *BuilderName.ToString())

	if (SavedBuilder)
	{
		UE_LOG(unDAWDataLogs, Verbose, TEXT("Found Builder %s"), *BuilderName.ToString())
		
		if (bResetBuilder)
		{
			UE_LOG(unDAWDataLogs, Verbose, TEXT("Resetting Builder %s"), *BuilderName.ToString())
			MSBuilderSystem->UnregisterBuilder(BuilderName);
		}
		else
		{
			BuilderContext = Cast<UMetaSoundSourceBuilder>(SavedBuilder);
			return;
		}
	}

	UE_LOG(unDAWDataLogs, Verbose, TEXT("Creating Builder %s"), *BuilderName.ToString())


	BuilderContext = MSBuilderSystem->CreateSourceBuilder(BuilderName, CoreNodes.OnPlayOutputNode, CoreNodes.OnFinishedNodeInput, CoreNodes.AudioOuts, BuildResult, MasterOptions.OutputFormat, false);
	CoreNodes.BuilderResults.Add(FName(TEXT("Create Builder")), BuildResult);
	CoreNodes.InitCoreNodes(BuilderContext, this);

	//iterate over vertexes and create the nodes
	for(auto& Vertex : Vertexes)
	{
		Vertex->BuildVertex();
		Vertex->CollectParamsForAutoConnect();
		//CoreNodes.BuilderResults.Add(FName(TEXT("Create Vertex Node")), BuildResult);
	}

	for (auto& Vertex : Vertexes)
	{
		Vertex->UpdateConnections();
	}
	//CoreNodes.CreateMidiPlayerAndMainClock(BuilderContext);

	MSBuilderSystem->RegisterSourceBuilder(BuilderName, BuilderContext);

}

void UDAWSequencerData::BeginDestroy()
{
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Destroying Sequencer Data"))

	bool MarkedDelete = RootPackageHasAnyFlags(RF_MarkAsRootSet);


	Super::BeginDestroy();
	

}

bool UDAWSequencerData::AuditionBuilder(UAudioComponent* InAuditionComponent, bool bForceRebuild)
{
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Auditioning Builder"))
	FOnCreateAuditionGeneratorHandleDelegate OnCreateAuditionGeneratorHandle;
	OnCreateAuditionGeneratorHandle.BindUFunction(this, TEXT("OnMetaSoundGeneratorHandleCreated"));
	AuditionComponent = InAuditionComponent;
	if (bForceRebuild) FindOrCreateBuilderForAsset(true);

	if (!BuilderContext) FindOrCreateBuilderForAsset(false);
	BuilderContext->Audition(this, InAuditionComponent, OnCreateAuditionGeneratorHandle, true);


	return true;
}

//UM2SoundGraphRenderer* UDAWSequencerData::CreatePerformer(UAudioComponent* InAuditionComponent)
//{
//	auto SequencerPerformer = NewObject<UM2SoundGraphRenderer>(this);
//
//	//OnVertexAdded.AddDynamic(SequencerPerformer, &UM2SoundGraphRenderer::UpdateVertex);
//	SequencerPerformer->InitPerformer();
//
//	SequencerPerformer->OutputFormat = MasterOptions.OutputFormat;
//
//	return SequencerPerformer;
//}


#if WITH_EDITOR
void UDAWSequencerData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bool bIsDirty = false;
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == TEXT("HarmonixMidiFile"))
	{
		PopulateFromMidiFile(HarmonixMidiFile);
		OnMidiDataChanged.Broadcast();
	}

	//if (PropertyName == TEXT("MasterVolume"))
	//{
	//	if (EditorPreviewPerformer && EditorPreviewPerformer->AuditionComponentRef)
	//	{
	//		EditorPreviewPerformer->AuditionComponentRef->SetVolumeMultiplier(MasterOptions.MasterVolume);
	//	}
	//}

	UE_LOG(unDAWDataLogs, Verbose, TEXT("Property Changed %s"), *PropertyName.ToString())
}

#endif

void UDAWSequencerData::PushPendingNotesToNewMidiFile()
{
	IsRecreatingMidiFile = true;

	auto MidiFileCopy = NewObject<UEditableMidiFile>(this);
	MidiFileCopy->LoadFromHarmonixBaseFile(HarmonixMidiFile);


	for(auto PendingNote : PendingLinkedMidiNotesMap)
	{
		//auto NewTrack = MidiFileCopy->AddTrack(FString::Printf(TEXT("Track %d"), TrackIndex++));
		auto TrackMetaData = GetTracksDisplayOptions(PendingNote.TrackId);

		UE_LOG(unDAWDataLogs, Verbose, TEXT("Pushing note to track %d, channel %d"), TrackMetaData.TrackIndexInParentMidi, TrackMetaData.ChannelIndexInParentMidi)

		auto TargetTrack = MidiFileCopy->GetTrack(TrackMetaData.TrackIndexInParentMidi);
		auto StartMessage = FMidiMsg::CreateNoteOn(TrackMetaData.ChannelIndexInParentMidi, PendingNote.pitch, 127);
		auto EndMessage = FMidiMsg::CreateNoteOff(TrackMetaData.ChannelIndexInParentMidi, PendingNote.pitch);
		auto NewStartNoteMidiEvent = FMidiEvent(PendingNote.StartTick, StartMessage);
		auto NewEndNoteMidiEvent = FMidiEvent(PendingNote.EndTick, EndMessage);

		TargetTrack->AddEvent(NewStartNoteMidiEvent);
		TargetTrack->AddEvent(NewEndNoteMidiEvent);

	}

	MidiFileCopy->SortAllTracks();

	PendingLinkedMidiNotesMap.Empty();
	PopulateFromMidiFile(MidiFileCopy);
	//HarmonixMidiFile = MidiFileCopy;
	//MidiFileCopy->operator new
		//HarmonixMidiFile
}


FAssignableAudioOutput FM2SoundCoreNodesComposite::GetFreeMasterMixerAudioOutput(UMetaSoundSourceBuilder* BuilderContext)
{
	if (MasterOutputs.Num() > 1)
	{
		return MasterOutputs.Pop();
	}
	else
	{
		ResizeOutputMixer(BuilderContext);
		return GetFreeMasterMixerAudioOutput(BuilderContext);
	}
}

void FM2SoundCoreNodesComposite::InitCoreNodes(UMetaSoundSourceBuilder* BuilderContext, UDAWSequencerData* ParentSession)
{
	//populate midi filter document
	FSoftObjectPath MidiFilterAssetRef(TEXT("/unDAW/Patches/System/unDAW_MidiFilter.unDAW_MidiFilter"));

	EMetaSoundBuilderResult BuildResult;
	SessionData = ParentSession;
	
	MidiFilterDocument = MidiFilterAssetRef.TryLoad();
	BuilderContext->AddInterface(FName(TEXT("unDAW Session Renderer")), BuildResult);
	BuilderResults.Add(FName(TEXT("Add unDAW Session Renderer Interface")), BuildResult);

	CreateMidiPlayerAndMainClock(BuilderContext);
	CreateMainMixer(BuilderContext);
}

void FM2SoundCoreNodesComposite::CreateMidiPlayerAndMainClock(UMetaSoundSourceBuilder* BuilderContext)
{
	// Create the midi player block
	EMetaSoundBuilderResult BuildResult;
	FSoftObjectPath MidiPlayerAssetRef(TEXT("/unDAW/Patches/System/unDAW_MainClock.unDAW_MainClock"));
	TScriptInterface<IMetaSoundDocumentInterface> MidiPlayerDocument = MidiPlayerAssetRef.TryLoad();
	UMetaSoundPatch* MidiPatch = Cast<UMetaSoundPatch>(MidiPlayerDocument.GetObject());

	MidiPlayerNode = BuilderContext->AddNode(MidiPlayerDocument, BuildResult);
	BuilderResults.Add(FName(TEXT("Add Harmonix Midi Player Node")), BuildResult);

	//connect inputs via interface bindings
	BuilderContext->ConnectNodeInputsToMatchingGraphInterfaceInputs(MidiPlayerNode, BuildResult);
	BuilderResults.Add(FName(TEXT("Connect Midi Player to unDAW Main Interface Inputs")), BuildResult);

	BuilderContext->ConnectNodeOutputsToMatchingGraphInterfaceOutputs(MidiPlayerNode, BuildResult);
	BuilderResults.Add(FName(TEXT("Connect Midi Player to unDAW Main Interface Outputs")), BuildResult);



	FMetaSoundBuilderNodeInputHandle MidiFileInput = BuilderContext->FindNodeInputByName(MidiPlayerNode, FName(TEXT("MIDI File")), BuildResult);
	MainMidiStreamOutput = BuilderContext->FindNodeOutputByName(MidiPlayerNode, FName(TEXT("unDAW.Midi Stream")), BuildResult);
	auto MidiInputPinOutputHandle = BuilderContext->AddGraphInputNode(TEXT("Midi File"), TEXT("MidiAsset"), SessionData->MSBuilderSystem->CreateObjectMetaSoundLiteral(SessionData->HarmonixMidiFile), BuildResult);
	BuilderContext->ConnectNodes(MidiInputPinOutputHandle, MidiFileInput, BuildResult);

}

void FM2SoundCoreNodesComposite::CreateMainMixer(UMetaSoundSourceBuilder* BuilderContext)
{
	// create master mixer
	EMetaSoundBuilderResult BuildResult;
	const auto MasterMixerNode = BuilderContext->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
		, BuildResult);

	auto MixerOutputs = BuilderContext->FindNodeOutputs(MasterMixerNode, BuildResult);
	//auto AudioOuts = GetAvailableOutput();
	bool usedLeft = false;
	for (const auto& Output : MixerOutputs)
	{
		FName NodeName;
		FName DataType;
		BuilderContext->GetNodeOutputData(Output, NodeName, DataType, BuildResult);
		if (DataType == FName(TEXT("Audio")))
		{
			BuilderContext->ConnectNodes(Output, AudioOuts[usedLeft ? 1 : 0], BuildResult);
			BuilderResults.Add(FName(FString::Printf(TEXT("Connect Main Mixer to Audio Out %s"), usedLeft ? TEXT("Right") : TEXT("Left"))), BuildResult);
			usedLeft = true;

		}
	}

	UM2SoundGraphStatics::PopulateAssignableOutputsArray(MasterOutputs, BuilderContext->FindNodeInputs(MasterMixerNode, BuildResult));
}

void FM2SoundCoreNodesComposite::ResizeOutputMixer(UMetaSoundSourceBuilder* BuilderContext)
{
	EMetaSoundBuilderResult BuildResult;
	const auto MasterMixerNode = BuilderContext->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
		, BuildResult);

	auto MixerOutputs = BuilderContext->FindNodeOutputs(MasterMixerNode, BuildResult);
	//auto AudioOuts = GetAvailableOutput();
	UE_LOG(LogTemp, Log, TEXT("Master Outputs Num: %d"), MasterOutputs.Num())
		auto LastRemainingOutput = MasterOutputs.Pop();
	BuilderContext->ConnectNodes(MixerOutputs[1], LastRemainingOutput.AudioLeftOutputInputHandle, BuildResult);
	BuilderContext->ConnectNodes(MixerOutputs[0], LastRemainingOutput.AudioRightOutputInputHandle, BuildResult);

	//PopulateAssignableOutputsArray(MasterOutputs, BuilderContext->FindNodeInputs(MasterMixerNode, BuildResult));

	UM2SoundGraphStatics::PopulateAssignableOutputsArray(MasterOutputs, BuilderContext->FindNodeInputs(MasterMixerNode, BuildResult));
}
