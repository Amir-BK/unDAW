// Fill out your copyright notice in the Description page of Project Settings.

#include "M2SoundGraphData.h"
#include "M2SoundGraphStatics.h"
#include "Metasound.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"
#include "Vertexes/M2SoundVertex.h"
#include "Components/AudioComponent.h"
#include "unDAWSettings.h"
#include "MetasoundSource.h"
#include <EditableMidiFile.h>
#include <HarmonixMidi/Blueprint/MidiNote.h>
#include <HarmonixMetasound/DataTypes/MidiEventInfo.h>
#include <Vertexes/M2VariMixerVertex.h>

DEFINE_LOG_CATEGORY(unDAWDataLogs);

#define AudioPinCast(Pin) Cast<UM2AudioTrackPin>(Pin)
#define LiteralCast(Pin) Cast<UM2MetasoundLiteralPin>(Pin)

struct FEventsWithIndex
{
	FMidiEvent Event;
	int32 EventIndex;
};

void UDAWSequencerData::CreateNewPatchBuilder()
{
	EMetaSoundBuilderResult BuildResult;
	PatchBuilder = MSBuilderSystem->CreatePatchBuilder(FName(TEXT("Test")), BuildResult);


	FMetaSoundBuilderOptions Options;
	Options.bAddToRegistry = true;
	Options.bForceUniqueClassName = true;
	Options.Name = FName(TEXT("Test123"));
	BuilderPatch = Cast<UMetaSoundPatch>(PatchBuilder->Build(nullptr, Options).GetObject());


}

void UDAWSequencerData::CreateDefaultVertexes()
{
	auto DefaultPatchTest = FSoftObjectPath(TEXT("'/unDAW/Patches/System/unDAW_Fusion_Piano.unDAW_Fusion_Piano'"));
	auto DefaultPatch = CastChecked<UMetaSoundPatch>(DefaultPatchTest.TryLoad());
	//we need to create an audio output and a vari mixer and connect them, this needs to be done even for empty daw sequencer files.
	// the output cannot be deleted from the graph, so we can just create it and connect it to the mixer
	auto NewOutput = FVertexCreator::CreateVertex<UM2SoundAudioOutput>(this);
	AddVertex(NewOutput);

	auto NewMixer = FVertexCreator::CreateVertex<UM2VariMixerVertex>(this);
	AddVertex(NewMixer);

	TArray<TObjectPtr<UM2Pins>> MixerInputPins;

	NewMixer->InputM2SoundPins.GenerateValueArray(MixerInputPins);

	ConnectPins<UM2AudioTrackPin>(AudioPinCast(NewOutput->InputM2SoundPins[M2Sound::Pins::AutoDiscovery::AudioTrack]), AudioPinCast(NewMixer->OutputM2SoundPins[M2Sound::Pins::AutoDiscovery::AudioTrack]));

	for (const auto& [Name, Input] : CoreNodes.MemberInputMap)
	{
		if (Input.DataType == "MIDIStream" && Input.MetadataIndex != INDEX_NONE)
		{
			UE_LOG(unDAWDataLogs, Verbose, TEXT("Creating MIDI Stream Node for %s"), *Name.ToString())
				auto NewInput = FVertexCreator::CreateVertex<UM2SoundBuilderInputHandleVertex>(this);
			NewInput->MemberName = Name;
			NewInput->TrackId = Input.MetadataIndex;

			AddVertex(NewInput);

			auto NewInstrument = FVertexCreator::CreateVertex<UM2SoundPatch>(this);
			NewInstrument->Patch = DefaultPatch;
			NewInstrument->TrackId = Input.MetadataIndex; //really this just affects the default graph creation, doesn't need to be set by default

			AddVertex(NewInstrument);

			auto InstrumentAudioOutput = AudioPinCast(NewInstrument->OutputM2SoundPins[M2Sound::Pins::AutoDiscovery::AudioTrack]);
			auto MixerInputPin = AudioPinCast(MixerInputPins.Pop());

			ConnectPins<UM2AudioTrackPin>(MixerInputPin, InstrumentAudioOutput);

			auto InstrumentLiteralMidiInput = LiteralCast(NewInstrument->InputM2SoundPins[FName(TEXT("unDAW Instrument.MidiStream"))]);
			auto NewInputLiteralMidiOutput = LiteralCast(NewInput->OutputM2SoundPins[FName(TEXT("MidiStream"))]);

			ConnectPins<UM2MetasoundLiteralPin>(InstrumentLiteralMidiInput, NewInputLiteralMidiOutput);
		}
	}
}


bool UDAWSequencerData::TraverseOutputPins(UM2SoundVertex* Vertex, TFunction<bool(UM2SoundVertex*)> Predicate)
{
	for (const auto& [Name, Pin] : Vertex->OutputM2SoundPins)
	{
		if (Predicate(Pin->ParentVertex)) return true;

		if(Pin->LinkedPin == nullptr) continue;

		if (TraverseOutputPins(Pin->LinkedPin->ParentVertex, Predicate)) return true;
	}
	return false;
}

bool UDAWSequencerData::TraverseInputPins(UM2SoundVertex* Vertex, TFunction<bool(UM2SoundVertex*)> Predicate)
{
	for (const auto& [Name, Pin] : Vertex->InputM2SoundPins)
	{
		if (Predicate(Pin->ParentVertex)) return true;
		
		if (Pin->LinkedPin == nullptr) continue;

		if (TraverseInputPins(Pin->LinkedPin->ParentVertex, Predicate)) return true;
	}

	return false;
}


bool UDAWSequencerData::WillConnectionCauseLoop(UM2SoundVertex* InInput, UM2SoundVertex* InOutput)
{
	return TraverseInputPins(InInput, [InOutput](UM2SoundVertex* Vertex) { return Vertex == InOutput; });
	//return TraverseOutputPins(InOutput, [InInput](UM2SoundVertex* Vertex) { return Vertex == InInput; });
}

void UDAWSequencerData::SaveDebugMidiFileTest()
{
	auto UniqueID = FGuid::NewGuid().ToString() + TEXT(".mid");
	HarmonixMidiFile->SaveStdMidiFile(FPaths::ProjectContentDir() / UniqueID);
}

void UDAWSequencerData::RebuildVertex(UM2SoundVertex* Vertex)
{
	Vertex->BuildVertex();
	Vertex->CollectParamsForAutoConnect();
	Vertex->UpdateConnections();
	Vertex->OnVertexUpdated.Broadcast();
}

void UDAWSequencerData::UpdateVertexConnections(UM2SoundVertex* Vertex)
{
	Vertex->UpdateConnections();
}

void UDAWSequencerData::ReceiveAudioParameter(FAudioParameter Parameter)
{
	if (AuditionComponent) AuditionComponent->SetParameter(MoveTemp(Parameter));
}

void UDAWSequencerData::ExecuteTriggerParameter(FName ParameterName)
{
	if (AuditionComponent) AuditionComponent->SetTriggerParameter(ParameterName);
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

void UDAWSequencerData::SetLoopSettings(const bool& InbIsLooping, const int32& BarDuration)
{
	EMetaSoundBuilderResult BuildResult;
	FName DataTypeName;
	CoreNodes.bIsLooping = InbIsLooping;
	auto SimpleLoopBoolInput = BuilderContext->FindNodeInputByName(CoreNodes.MidiPlayerNode, FName(TEXT("Simple Loop")), BuildResult);
	CoreNodes.BuilderResults.Add(FName(TEXT("Find Simple Loop Input")), BuildResult);
	BuilderContext->SetNodeInputDefault(SimpleLoopBoolInput, MSBuilderSystem->CreateBoolMetaSoundLiteral(InbIsLooping, DataTypeName), BuildResult);
	CoreNodes.BuilderResults.Add(FName(TEXT("Set Simple Loop Input")), BuildResult);

	if (BarDuration != INDEX_NONE) HarmonixMidiFile->GetSongMaps()->SetLengthTotalBars(BarDuration); //small hack, for now, so we don't overwrite duration if not needed
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Setting Loop Settings %s, %d"), InbIsLooping ? TEXT("True") : TEXT("False"), BarDuration)
}

void UDAWSequencerData::ReceiveMetaSoundMidiStreamOutput(FName OutputName, const FMetaSoundOutput Value)
{
	FMidiEventInfo MidiEvent;

	Value.Get(MidiEvent);

	//find the metadata index for this event
	auto FoundTrack = M2TrackMetadata.IndexOfByPredicate([MidiEvent](const FTrackDisplayOptions& Track) {
		return Track.ChannelIndexRaw == MidiEvent.GetChannel() && MidiEvent.TrackIndex == Track.TrackIndexInParentMidi;
		});

	if (FoundTrack == INDEX_NONE) return;

	//if is note on, add to map with found index, else look in map for note with same number and remove it

	if (MidiEvent.IsNote())
	{
		if (MidiEvent.IsNoteOn())
		{
			CurrentlyActiveNotes.Add(TTuple<int, int>(FoundTrack, MidiEvent.GetNoteNumber()));
		}
		else {
			CurrentlyActiveNotes.Remove(TTuple<int, int>(FoundTrack, MidiEvent.GetNoteNumber()));
		}
	}

	//auto ValueType = Value.GetDataTypeName();
	//UE_LOG(unDAWDataLogs, Verbose, TEXT("Received Midi Stream Output %s, %s"), *OutputName.ToString(), *ValueType.ToString())

	//UE_LOG(unDAWDataLogs, Verbose, TEXT("Received Midi Stream Output %s, %d, %d"), *OutputName.ToString(), MidiEvent.GetMsg().GetStdData1(), MidiEvent.GetMsg().GetStdData2())
}

void UDAWSequencerData::ReceiveMetaSoundMidiClockOutput(FName OutputName, const FMetaSoundOutput Value)
{
	Value.Get(CurrentTimestampData);

	OnTimeStampUpdated.Broadcast(CurrentTimestampData);
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

	SavedMetaSound = Cast<UMetaSoundSource>(AuditionComponent->Sound);

	OnBuilderReady.Broadcast();
}

void UDAWSequencerData::SendTransportCommand(EBKTransportCommands Command)
{
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Received transport Command, Current Playback State %s"), *UEnum::GetValueAsString(PlayState))

		auto CurrentPlaystate = PlayState;

	if (AuditionComponent != nullptr)
	{
		switch (Command)
		{
		case EBKTransportCommands::Pause:
		case EBKTransportCommands::Play:

			switch (PlayState)
			{
			case EBKPlayState::TransportPlaying:
				AuditionComponent->SetTriggerParameter(FName(TEXT("unDAW.Transport.Pause")));
				PlayState = EBKPlayState::TransportPaused;
				break;

			case EBKPlayState::ReadyToPlay:
			case EBKPlayState::TransportPaused:
				AuditionComponent->SetTriggerParameter(FName(TEXT("unDAW.Transport.Play")));
				PlayState = EBKPlayState::TransportPlaying;
				break;
			}

			break;
		case EBKTransportCommands::Stop:
			AuditionComponent->SetTriggerParameter(FName(TEXT("unDAW.Transport.Stop")));
			//set time to 0
			CurrentTimestampData.Reset();
			PlayState = EBKPlayState::ReadyToPlay;
			SendSeekCommand(0.f);
			//OnTimeStampUpdated.Broadcast(CurrentTimestampData);
			break;

		default:
			break;
		}
	}

	if (PlayState != CurrentPlaystate) OnPlaybackStateChanged.Broadcast(PlayState);
}

void UDAWSequencerData::SendSeekCommand(float InSeek)
{
	UE_LOG(LogTemp, Log, TEXT("Seek Command Received! %f"), InSeek)
		if (AuditionComponent)
		{
			AuditionComponent->SetFloatParameter(FName(TEXT("unDAW.Transport.SeekTarget")), InSeek * 1000.f);
			AuditionComponent->SetTriggerParameter(FName(TEXT("unDAW.Transport.Seek")));
			CurrentlyActiveNotes.Empty();
		}
}

void UDAWSequencerData::PrintMidiData()
{
	UE_LOG(unDAWDataLogs, Log, TEXT("Printing Midi Data (unDAW Current Copy)"))
		for (auto& [Channel, LinkedNotes] : LinkedNoteDataMap)
		{
			UE_LOG(unDAWDataLogs, Verbose, TEXT("Channel %d"), Channel)
				for (auto& Note : LinkedNotes.LinkedNotes)
				{
					UE_LOG(unDAWDataLogs, Verbose, TEXT("Note %d, Start %d, End %d, [TR: %d, CH: %d]"), Note.pitch, Note.StartTick, Note.EndTick, Note.TrackId, Note.ChannelId)
				}
		}

	//tracks in metadata
	UE_LOG(unDAWDataLogs, Log, TEXT("Tracks in unDAW Metadata"))
		for (auto& Track : M2TrackMetadata)
		{
			UE_LOG(unDAWDataLogs, Log, TEXT("Track %s"), *Track.trackName)
		}

	//tracks in midi file
	UE_LOG(unDAWDataLogs, Log, TEXT("Tracks in midi file"))
		for (auto& Track : HarmonixMidiFile->GetTracks())
		{
			UE_LOG(unDAWDataLogs, Log, TEXT("Track %s"), *FString(*Track.GetName()))
		}
}

FString GetUniqueNameForTrack(const FString& BaseName, const UMidiFile& MidiFile)
{
	FString NewName = BaseName;
	int TrackIndex = 0;
	while (MidiFile.FindTrackIndexByName(NewName) != INDEX_NONE)
	{
		NewName = FString::Printf(TEXT("%s %d"), *BaseName, TrackIndex++);
	}
	return NewName;
}

void UDAWSequencerData::AddTrack()
{
	auto MidiFileCopy = NewObject<UEditableMidiFile>(this);
	MidiFileCopy->LoadFromHarmonixBaseFile(HarmonixMidiFile);

	//add track at the end of metadata array, ensure widget is updated
	FTrackDisplayOptions NewTrackMetaData;
	NewTrackMetaData.ChannelIndexInParentMidi = 0;
	NewTrackMetaData.ChannelIndexRaw = 0;
	NewTrackMetaData.trackName = GetUniqueNameForTrack(TEXT("New Track"), *MidiFileCopy);
	NewTrackMetaData.fusionPatch = nullptr;
	NewTrackMetaData.trackColor = FLinearColor::MakeRandomColor();

	auto NewTrack = MidiFileCopy->AddTrack(NewTrackMetaData.trackName);

	//auto TrackIndexByName = HarmonixMidiFile->FindTrackIndexByName(NewTrackMetaData.trackName);
	NewTrackMetaData.TrackIndexInParentMidi = MidiFileCopy->GetTracks().Num() - 1;

	//HarmonixMidiFile->SortAllTracks();
	M2TrackMetadata.Add(NewTrackMetaData);
	CoreNodes.CreateFilterNodeForTrack(M2TrackMetadata.Num() - 1);
	//UM2SoundGraphStatics::CreateDefaultVertexesFromInputData(this, M2TrackMetadata.Num() - 1);

	HarmonixMidiFile = MidiFileCopy;

	OnMidiDataChanged.Broadcast();
}

void UDAWSequencerData::ReinitGraph()
{
	FindOrCreateBuilderForAsset(true);
#if WITH_EDITOR
	if (M2SoundGraph)
	{
		M2SoundGraph->InitializeGraph();
	}
#endif
}

void UDAWSequencerData::AddVertex(UM2SoundVertex* Vertex)
{
	Vertexes.Add(Vertex);
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Adding Vertex %s"), *Vertex->GetName())
		//update builder, in the future we may want to be a little bit more conservative with this
		//MSBuilderSystem->RegisterBuilder(BuilderName, BuilderContext);
		Vertex->SequencerData = this;

	if (!BuilderContext) return;

	Vertex->BuildVertex();
	Vertex->CollectParamsForAutoConnect();
	Vertex->UpdateConnections();
	Vertex->OnVertexUpdated.Broadcast();
}

inline void UDAWSequencerData::InitMetadataFromFoundMidiTracks(TArray<TTuple<int, int>> InTracks) {
	auto PianoPatchPath = FSoftObjectPath(TEXT("/Harmonix/Examples/Patches/Piano.Piano"));
	//Vertexes.Empty();

	UFusionPatch* PianoPatch = static_cast<UFusionPatch*>(PianoPatchPath.TryLoad());

	M2TrackMetadata.Empty();
	for (const auto& [trackID, channelID] : InTracks)
	{
		FTrackDisplayOptions newTrack;
		bool bIsPrimaryChannel = HarmonixMidiFile->GetTrack(trackID)->GetPrimaryMidiChannel() == channelID;
		newTrack.ChannelIndexInParentMidi = bIsPrimaryChannel ? 0 : channelID;
		newTrack.ChannelIndexRaw = channelID;
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
			trackColor = FLinearColor::Yellow;
			break;
		case 2:
			trackColor = FLinearColor::Green;
			break;

		default:
			trackColor = FLinearColor::MakeRandomSeededColor(channelID * 16 + trackID);
			break;
		}
		M2TrackMetadata[IndexOfNewTrack].trackColor = trackColor;

		//So actually here we can create the Midi Stream Core Nodes...

		//AddVertex(NewInput);
		//TrackInputs.Add(IndexOfNewTrack, NewInput);
		// So we actually need to do the whole vertex init only after corenode creation and metadata discovery is complete...
		// Disable for now, this is an opportunity to refactor
		//UM2SoundGraphStatics::CreateDefaultVertexesFromInputData(this, IndexOfNewTrack);
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

	UE_LOG(unDAWDataLogs, Verbose, TEXT("Pushing note to track %d, channel %d"), TrackMetaData.TrackIndexInParentMidi, TrackMetaData.ChannelIndexRaw)

		// why - 1 ? ffs.
		//auto TargetTrackIndex = MidiFileCopy->FindTrackIndexByName(TrackMetaData.trackName);
		auto TargetTrack = MidiFileCopy->GetTrack(TrackMetaData.TrackIndexInParentMidi);

	if (!TargetTrack)
	{
		UE_LOG(unDAWDataLogs, Error, TEXT("Target Track not found!"))
			return;
	}

	auto StartMessage = FMidiMsg::CreateNoteOn(TrackMetaData.ChannelIndexRaw, PendingNote.pitch, PendingNote.NoteVelocity);
	auto EndMessage = FMidiMsg::CreateNoteOff(TrackMetaData.ChannelIndexRaw, PendingNote.pitch);
	auto NewStartNoteMidiEvent = FMidiEvent(PendingNote.StartTick, StartMessage);
	auto NewEndNoteMidiEvent = FMidiEvent(PendingNote.EndTick, EndMessage);

	TargetTrack->AddEvent(NewStartNoteMidiEvent);
	TargetTrack->AddEvent(NewEndNoteMidiEvent);

	PendingLinkedMidiNotesMap.Add(PendingNote);
	MidiFileCopy->SortAllTracks();
	//auto LastEventTick = MidiFileCopy->GetLastEventTick();
	//MidiFileCopy->GetSongMaps()->SetLengthTotalBars(4);
	//MidiFileCopy->LoopBarDuration = 4;

	HarmonixMidiFile = MidiFileCopy;
	MarkPackageDirty();

	if (AuditionComponent) AuditionComponent->SetObjectParameter(FName(TEXT("Midi File")), HarmonixMidiFile);
}

void UDAWSequencerData::DeleteLinkedMidiEvent(FLinkedMidiEvents PendingNote)
{
	//so, before we can delete the note we need to empty our pending notes map and repopulate the main data map
	// no actually, we can just remove the note from the midi file first
	auto MidiFileCopy = NewObject<UEditableMidiFile>(this);
	MidiFileCopy->LoadFromHarmonixBaseFile(HarmonixMidiFile);

	auto& NoteMetadata = M2TrackMetadata[PendingNote.TrackId];
	PendingNote.ChannelId = NoteMetadata.ChannelIndexRaw;

	UE_LOG(unDAWDataLogs, Verbose, TEXT("Deleting note from track %d, channel %d"), PendingNote.TrackId, PendingNote.ChannelId)
		//print pending note data
		UE_LOG(unDAWDataLogs, Verbose, TEXT("Pending Note Data: Start %d, End %d, Pitch %d, Channel %d"), PendingNote.StartTick, PendingNote.EndTick, PendingNote.pitch, PendingNote.ChannelId)

		auto FoundStartEvent = MidiFileCopy->GetTrack(NoteMetadata.TrackIndexInParentMidi)->GetEvents().IndexOfByPredicate([PendingNote](const FMidiEvent& Event) {
		// print event tick and data
		if (!Event.GetMsg().IsNoteOn()) return false;

		UE_LOG(unDAWDataLogs, Verbose, TEXT("Delete note on Event Tick %d, Data1 %d, Data2 %d, Channel %d"), Event.GetTick(), Event.GetMsg().GetStdData1(), Event.GetMsg().GetStdData2(), Event.GetMsg().GetStdChannel())
			if (Event.GetTick() == PendingNote.StartTick && Event.GetMsg().GetStdData1() == PendingNote.pitch && Event.GetMsg().GetStdChannel() == PendingNote.ChannelId)
			{
				return true;
			}
		return false;
			});

	//we need to find the real end event, not the one we have in the pending notes map

	auto FoundEndEvent = MidiFileCopy->GetTrack(NoteMetadata.TrackIndexInParentMidi)->GetEvents().IndexOfByPredicate([PendingNote](const FMidiEvent& Event) {
		//if (Event.GetTick() == PendingNote.EndTick) return true;

		if (!Event.GetMsg().IsNoteOff()) return false;

		UE_LOG(unDAWDataLogs, Verbose, TEXT("Delete note off Event Tick %d, Data1 %d, Data2 %d, Channel: %d"), Event.GetTick(), Event.GetMsg().GetStdData1(), Event.GetMsg().GetStdData2(), Event.GetMsg().GetStdChannel())
			//start with comparing the tick and the channel...
			if (Event.GetTick() == PendingNote.EndTick && Event.GetMsg().GetStdData1() == PendingNote.pitch && Event.GetMsg().GetStdChannel() == PendingNote.ChannelId)
			{
				return true;
			}

		return false;
		});

	//print indexes
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Found Start Event %d, Found End Event %d"), FoundStartEvent, FoundEndEvent)

		if (FoundStartEvent != INDEX_NONE && FoundEndEvent != INDEX_NONE)
		{
			UE_LOG(unDAWDataLogs, Verbose, TEXT("Found note to delete!"))
				MidiFileCopy->GetTrack(NoteMetadata.TrackIndexInParentMidi)->GetRawEvents().RemoveAt(FoundEndEvent, 1, false);
			MidiFileCopy->GetTrack(NoteMetadata.TrackIndexInParentMidi)->GetRawEvents().RemoveAt(FoundStartEvent, 1, false);
		}
		else {
			UE_LOG(unDAWDataLogs, Error, TEXT("Couldn't find note to delete!"))
				return;
		}
	MidiFileCopy->SortAllTracks();
	HarmonixMidiFile = MidiFileCopy;

	auto FoundChannelsArray = TArray<TTuple<int, int>>();

	UpdateNoteDataFromMidiFile(FoundChannelsArray);
}

//TODO: I don't like this implementation, the linked notes should be created by demand from the midifile and only stored transiently
void UDAWSequencerData::PopulateFromMidiFile(UMidiFile* inMidiFile)
{
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Populating Sequencer Data from Midi File"))

		TArray<TTuple<int, int>> FoundChannels;
	//LinkedNoteDataMap.Empty();
	//PendingLinkedMidiNotesMap.Empty();
	HarmonixMidiFile = inMidiFile;
	UpdateNoteDataFromMidiFile(FoundChannels);

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

	//MidiSongMap = HarmonixMidiFile->GetSongMaps();

	CalculateSequenceDuration();
	//if we're recreating the midi file we don't need to do anything else
	if (IsRecreatingMidiFile)
	{
		IsRecreatingMidiFile = false;
	}
	else {
		InitMetadataFromFoundMidiTracks(FoundChannels);
		FindOrCreateBuilderForAsset(true);
		CreateDefaultVertexes();
	}

#if WITH_EDITOR
	if (M2SoundGraph)
	{
		M2SoundGraph->InitializeGraph();
	}
#endif
}

void UDAWSequencerData::UpdateNoteDataFromMidiFile(TArray<TTuple<int, int>>& OutDiscoveredChannels)
{
	LinkedNoteDataMap.Empty();
	PendingLinkedMidiNotesMap.Empty();

	//if midi file is EMPTY we add a track (empty note track) and default tempo and time signature on track 0 (conductor)

	auto& FoundChannels = OutDiscoveredChannels;

	int numTracks = 0;
	int numTracksRaw = 0;
	BeatsPerMinute = HarmonixMidiFile->GetSongMaps()->GetTempoAtTick(0);

	for (auto& track : HarmonixMidiFile->GetTracks())
	{
		//if track has no events we can continue, but this never happens, it might not have note events but it has events.
		int numTracksInternal = numTracksRaw++;
		//if (track.GetEvents().IsEmpty()) continue;

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

						if (midiChannel == unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].Event.GetMsg().GetStdChannel())
						{
							FLinkedMidiEvents foundPair = FLinkedMidiEvents(unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].Event, MidiEvent,
								unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].EventIndex, index);
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
				TempoEventsMap.Add(MidiEvent.GetTick(), MidiEvent.GetMsg().GetMicrosecPerQuarterNote());
				break;
			case FMidiMsg::EType::TimeSig:
				//UE_LOG(unDAWDataLogs, Verbose, TEXT("We receive a time signature event!"))
				TimeSignatureEvents.Add(MidiEvent);
				TimeSignatureMap.Add(MidiEvent.GetTick(), FVector2f(MidiEvent.GetMsg().GetTimeSigNumerator(), MidiEvent.GetMsg().GetTimeSigDenominator()));
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

		//if (LinkedNoteDataMap.IsEmpty()) continue;

		//CreateBuilderHelper();
	}

	//hacky...
	if (FoundChannels.IsEmpty() && M2TrackMetadata.IsEmpty())
	{
		//create default track for now
		auto NewTrackMetaData = FTrackDisplayOptions();

		NewTrackMetaData.ChannelIndexInParentMidi = 0;
		NewTrackMetaData.TrackIndexInParentMidi = 0;
		NewTrackMetaData.trackName = TEXT("Default Track");
		NewTrackMetaData.fusionPatch = nullptr;
		NewTrackMetaData.trackColor = FLinearColor::Red;
		//

		M2TrackMetadata.Add(NewTrackMetaData);
	}
}

void UDAWSequencerData::FindOrCreateBuilderForAsset(bool bResetBuilder)
{
	MSBuilderSystem = GEngine->GetEngineSubsystem<UMetaSoundBuilderSubsystem>();

	StructParametersPack = NewObject<UMetasoundParameterPack>(this);

	EMetaSoundBuilderResult BuildResult;

	auto SavedBuilder = MSBuilderSystem->FindBuilder(BuilderName);
	//MSBuilderSystem->
	//log SavedBuilder result

	if (SavedBuilder == nullptr) UE_LOG(unDAWDataLogs, Verbose, TEXT("Builder %s not found"), *BuilderName.ToString())

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
	SetLoopSettings(CoreNodes.bIsLooping, CoreNodes.BarLoopDuration);

	//iterate over vertexes and create the nodes
	for (auto& Vertex : Vertexes)
	{
		UE_LOG(unDAWDataLogs, Verbose, TEXT("Creating Vertex Node, Vertex Name %s"), *Vertex->GetName())
			Vertex->BuildVertex();
		Vertex->CollectParamsForAutoConnect();
		//CoreNodes.BuilderResults.Add(FName(TEXT("Create Vertex Node")), BuildResult);
	}

	for (auto& Vertex : Vertexes)
	{
		Vertex->UpdateConnections();
		//to init the graph to the last mix state
		Vertex->CollectAndTransmitAudioParameters();
	}
	//CoreNodes.CreateMidiPlayerAndMainClock(BuilderContext);

	MSBuilderSystem->RegisterSourceBuilder(BuilderName, BuilderContext);
}

void UDAWSequencerData::ApplyParameterPack(UMetasoundParameterPack* InPack)
{
	if(GeneratorHandle) GeneratorHandle->ApplyParameterPack(InPack);
}

void UDAWSequencerData::RemoveVertex(UM2SoundVertex* Vertex)
{
	Vertexes.Remove(Vertex);
	//Vertex->DestroyVertex();
	//Vertex->OnVertexDestroyed.Broadcast();
}

void UDAWSequencerData::BeginDestroy()
{
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Destroying Sequencer Data"))

		bool MarkedDelete = RootPackageHasAnyFlags(RF_MarkAsRootSet);

	Super::BeginDestroy();
}

void UDAWSequencerData::AuditionBuilder(UAudioComponent* InAuditionComponent, bool bForceRebuild)
{
	//if Midi file is empty don't audition

	if (HarmonixMidiFile->IsEmpty()) return;

	UE_LOG(unDAWDataLogs, Verbose, TEXT("Auditioning Builder"))
		FOnCreateAuditionGeneratorHandleDelegate OnCreateAuditionGeneratorHandle;
	OnCreateAuditionGeneratorHandle.BindUFunction(this, TEXT("OnMetaSoundGeneratorHandleCreated"));
	AuditionComponent = InAuditionComponent;
	//AuditionComponent->
	if (bForceRebuild) FindOrCreateBuilderForAsset(true);

	if (!BuilderContext) FindOrCreateBuilderForAsset(false);
	BuilderContext->Audition(this, InAuditionComponent, OnCreateAuditionGeneratorHandle, true);
	SavedMetaSound->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
	AuditionComponent->SetVolumeMultiplier(MasterOptions.MasterVolume);
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

	for (auto PendingNote : PendingLinkedMidiNotesMap)
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

FAssignableAudioOutput FM2SoundCoreNodesComposite::GetFreeMasterMixerAudioOutput()
{
	if (MasterOutputs.Num() > 1)
	{
		return MasterOutputs.Pop();
	}
	else
	{
		ResizeOutputMixer();
		return GetFreeMasterMixerAudioOutput();
	}
}

void FM2SoundCoreNodesComposite::ReleaseMasterMixerAudioOutput(FAssignableAudioOutput Output)
{
	MasterOutputs.Add(Output);

	EMetaSoundBuilderResult BuildResult;

	BuilderContext->DisconnectNodeInput(Output.AudioLeftOutputInputHandle, BuildResult);
	BuilderContext->DisconnectNodeInput(Output.AudioRightOutputInputHandle, BuildResult);
}

void FM2SoundCoreNodesComposite::MarkAllMemberInputsStale()
{
	for (auto& MemberInput : MemberInputs)
	{
		MemberInput.bIsStale = true;
	}
}

void FM2SoundCoreNodesComposite::RemoveAllStaleInputs()
{
	auto CopyOfMemberInputs = MemberInputs;

	//for(auto& MemberInput : CopyOfMemberInputs)
	//{
	//	if(MemberInput.bIsStale)
	//	{
	//		MemberInputs.Remove(MemberInput);
	//	}
	//}
}

void FM2SoundCoreNodesComposite::CreateOrUpdateMemberInput(FMetaSoundBuilderNodeOutputHandle InHandle, FName InName, int MetadataIndex)
{
	EMetaSoundBuilderResult BuildResult;

	FMemberInput NewMemberInput;
	NewMemberInput.MetadataIndex = MetadataIndex;
	BuilderContext->GetNodeOutputData(InHandle, NewMemberInput.Name, NewMemberInput.DataType, BuildResult);
	if (InName != NAME_None) NewMemberInput.Name = InName;

	if (MemberInputMap.Contains(NewMemberInput.Name))
	{
		auto& ExistingMemberInput = MemberInputMap[NewMemberInput.Name];
		ExistingMemberInput.SetMemberInputOutputHandle(InHandle, ExistingMemberInput.Name, ExistingMemberInput.DataType);
		//ExistingMemberInput.bIsStale = false;
	}
	else
	{
		NewMemberInput.SetMemberInputOutputHandle(InHandle, NewMemberInput.Name, NewMemberInput.DataType);
		//MemberInputs.Add(NewMemberInput);
		MemberInputMap.Add(NewMemberInput.Name, NewMemberInput);
	}
}

void FM2SoundCoreNodesComposite::InitCoreNodes(UMetaSoundSourceBuilder* InBuilderContext, UDAWSequencerData* ParentSession)
{
	//populate midi filter document
	FSoftObjectPath MidiFilterAssetRef(TEXT("/unDAW/Patches/System/unDAW_MidiFilter.unDAW_MidiFilter"));

	EMetaSoundBuilderResult BuildResult;
	SessionData = ParentSession;
	BuilderContext = InBuilderContext;

	MidiFilterDocument = MidiFilterAssetRef.TryLoad();
	BuilderContext->AddInterface(FName(TEXT("unDAW Session Renderer")), BuildResult);
	BuilderResults.Add(FName(TEXT("Add unDAW Session Renderer Interface")), BuildResult);

	CreateMidiPlayerAndMainClock();

	for (int32 TrackIndex = 0; TrackIndex < SessionData->M2TrackMetadata.Num(); TrackIndex++)
	{
		CreateFilterNodeForTrack(TrackIndex);
	}

	CreateMainMixer();
}

FMetaSoundBuilderNodeOutputHandle FM2SoundCoreNodesComposite::CreateFilterNodeForTrack(int32 TrackMetadataIndex)
{
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Creating Filter Node for Track %d"), TrackMetadataIndex)
		EMetaSoundBuilderResult BuildResult;

	FTrackDisplayOptions& TrackMetadata = SessionData->GetTracksDisplayOptions(TrackMetadataIndex);
	// ClassName=(Namespace="unDAW",Name="MidiStreamTrackIsolator")
	auto NewNode = BuilderContext->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("unDAW")), FName(TEXT("MidiStreamTrackIsolator"))), BuildResult, 1);
	auto NewMidiStreamOutput = BuilderContext->FindNodeOutputByName(NewNode, FName(TEXT("MidiStream")), BuildResult);
	//this guy only has one output, which is the MidiStream grab it and add it to MappedOutputs

	//MappedOutputs.Add(TrackMetadataIndex, FMemberInput{ FName(TrackMetadata.trackName), NewMidiStreamOutput});
	CreateOrUpdateMemberInput(NewMidiStreamOutput, FName(TrackMetadata.trackName + ".MidiStream"), TrackMetadataIndex);

	//set default values to "Track Index" and "Channel Index"
	auto TrackInput = BuilderContext->FindNodeInputByName(NewNode, FName(TEXT("Track Index")), BuildResult);
	auto ChannelInput = BuilderContext->FindNodeInputByName(NewNode, FName(TEXT("Channel Index")), BuildResult);

	FName DataTypeName;
	auto TrackIntLiteral = SessionData->MSBuilderSystem->CreateIntMetaSoundLiteral(TrackMetadata.TrackIndexInParentMidi, DataTypeName);
	auto ChannelIntLiteral = SessionData->MSBuilderSystem->CreateIntMetaSoundLiteral(TrackMetadata.ChannelIndexRaw, DataTypeName);

	BuilderContext->SetNodeInputDefault(TrackInput, TrackIntLiteral, BuildResult);
	BuilderContext->SetNodeInputDefault(ChannelInput, ChannelIntLiteral, BuildResult);

	//find midistream input and connect to main midi stream, which was already created

	auto MidiStreamInput = BuilderContext->FindNodeInputByName(NewNode, FName(TEXT("MidiStream")), BuildResult);
	BuilderContext->ConnectNodes(MainMidiStreamOutput, MidiStreamInput, BuildResult);

	//for now also connect to graph output with the same name as the metadata track name
	auto GraphOutput = BuilderContext->AddGraphOutputNode(FName(TrackMetadata.trackName), TEXT("MidiStream"), FMetasoundFrontendLiteral(), BuildResult);
	BuilderContext->ConnectNodes(NewMidiStreamOutput, GraphOutput, BuildResult);

	return NewMidiStreamOutput;
}

void FM2SoundCoreNodesComposite::CreateMidiPlayerAndMainClock()
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
	CreateOrUpdateMemberInput(MainMidiStreamOutput);
	//FMemberInput MainMidiStreamOutputHandle = FMemberInput();

	auto MainMidiClockOutput = BuilderContext->FindNodeOutputByName(MidiPlayerNode, FName(TEXT("unDAW.Midi Clock")), BuildResult);
	CreateOrUpdateMemberInput(MainMidiClockOutput);

	auto MainTransportOutput = BuilderContext->FindNodeOutputByName(MidiPlayerNode, FName(TEXT("Transport")), BuildResult);
	CreateOrUpdateMemberInput(MainTransportOutput);

	//MainMidiStreamOutputHandle.OutputHandle = MainMidiStreamOutput;
	//MainMidiStreamOutputHandle.OutputName = FName(TEXT("Main Midi Stream"));
	//MainMidiStreamOutputHandle.bGraphOutput = true;

	//MappedOutputs.Add(MainMidiStreamOutputHandle.OutputName, MainMidiStreamOutputHandle);

	auto MidiInputPinOutputHandle = BuilderContext->AddGraphInputNode(TEXT("Midi File"), TEXT("MidiAsset"), SessionData->MSBuilderSystem->CreateObjectMetaSoundLiteral(SessionData->HarmonixMidiFile), BuildResult);
	BuilderContext->ConnectNodes(MidiInputPinOutputHandle, MidiFileInput, BuildResult);
}

void FM2SoundCoreNodesComposite::CreateMainMixer()
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

void FM2SoundCoreNodesComposite::ResizeOutputMixer()
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

#undef AudioPinCast
#undef LiteralCast