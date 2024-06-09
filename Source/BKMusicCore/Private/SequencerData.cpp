// Fill out your copyright notice in the Description page of Project Settings.


#include "SequencerData.h"
#include "UnDAWSequencePerformer.h"
#include "M2SoundGraphStatics.h"
#include "Metasound.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"

DEFINE_LOG_CATEGORY(unDAWDataLogs);


struct FEventsWithIndex
{
	FMidiEvent event;
	int32 eventIndex;
};


	

void UDAWSequencerData::AddVertex(UM2SoundVertex* Vertex)
{
	
	//add patch or insert
	if(auto NewOutput = Cast<UM2SoundAudioOutput>(Vertex))
	{
		Outputs.Add(NewOutput->GetFName(), NewOutput);
	}

	if (auto NewPatch = Cast<UM2SoundPatch>(Vertex))
	{
		Patches.Add(NewPatch->GetFName(), NewPatch);
	}

	OnVertexAdded.Broadcast(Vertex);
}

void UDAWSequencerData::ChangeFusionPatchInTrack(int TrackID, UFusionPatch* NewPatch)
{
	if (M2TrackMetadata.IsValidIndex(TrackID))
	{
		M2TrackMetadata[TrackID].fusionPatch = NewPatch;
		OnFusionPatchChangedInTrack.ExecuteIfBound(TrackID, NewPatch);
	}

}

inline void UDAWSequencerData::InitVertexesFromFoundMidiTracks(TArray<TTuple<int, int>> InTracks) {

	auto PianoPatchPath = FSoftObjectPath(TEXT("/Harmonix/Examples/Patches/Piano.Piano"));

	UFusionPatch* PianoPatch = static_cast<UFusionPatch*>(PianoPatchPath.TryLoad());

	M2TrackMetadata.Empty();
	for (const auto& [trackID, channelID] : InTracks)
	{

		FTrackDisplayOptions newTrack;
		bool bIsPrimaryChannel = HarmonixMidiFile->GetTrack(trackID)->GetPrimaryMidiChannel() == channelID;
		newTrack.ChannelIndexInParentMidi = bIsPrimaryChannel ? 0 : channelID;
		newTrack.TrackIndexInParentMidi = trackID;

		newTrack.trackName = *HarmonixMidiFile->GetTrack(trackID)->GetName() + " Ch: " + FString::FromInt(channelID) + " Tr: " + FString::FromInt(trackID);
		newTrack.trackColor = FLinearColor::MakeRandomSeededColor(channelID * 16 + trackID);
		newTrack.fusionPatch = PianoPatch;
		int IndexOfNewTrack = M2TrackMetadata.Add(newTrack);

		UM2SoundTrackInput* NewInput = NewObject<UM2SoundTrackInput>(this, NAME_None, RF_Transactional);
		NewInput->SequencerData = this;

		NewInput->TrackId = IndexOfNewTrack;

		TrackInputs.Add(IndexOfNewTrack, NewInput);
		UM2SoundGraphStatics::CreateDefaultVertexesFromInputVertex(this, NewInput, IndexOfNewTrack);
	}

}

FTrackDisplayOptions& UDAWSequencerData::GetTracksDisplayOptions(const int& ID)
{
	if(M2TrackMetadata.IsValidIndex(ID))
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


//TODO: I don't like this implementation, the linked notes should be created by demand from the midifile and only stored transiently
void UDAWSequencerData::PopulateFromMidiFile(UMidiFile* inMidiFile)
{
	TArray<TTuple<int, int>> FoundChannels;
	LinkedNoteDataMap.Empty();
	Outputs.Empty();
	TrackInputs.Empty();
	Patches.Empty();
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

		//FoundChannels.Sort();
		CalculateSequenceDuration();
		InitVertexesFromFoundMidiTracks(FoundChannels);

		//we must create a builder to traverse the graphs and create the nodes
		CreatePerformer(nullptr);

#if WITH_EDITOR
		if(M2SoundGraph)
		{
			M2SoundGraph->InitializeGraph();
		}
#endif
		//CreateBuilderHelper();
	}
}

UM2SoundGraphRenderer* UDAWSequencerData::CreatePerformer(UAudioComponent* AuditionComponent)
{
	auto SequencerPerformer = NewObject<UM2SoundGraphRenderer>(this);

	OnVertexAdded.AddDynamic(SequencerPerformer, &UM2SoundGraphRenderer::UpdateVertex);
	SequencerPerformer->InitPerformer();

	SequencerPerformer->OutputFormat = MasterOptions.OutputFormat;

	return SequencerPerformer;
}

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

	if (PropertyName == TEXT("MasterVolume"))
	{
		if (EditorPreviewPerformer && EditorPreviewPerformer->AuditionComponentRef)
		{
			EditorPreviewPerformer->AuditionComponentRef->SetVolumeMultiplier(MasterOptions.MasterVolume);
		}

	}

	UE_LOG(unDAWDataLogs, Verbose, TEXT("Property Changed %s"), *PropertyName.ToString())
}
#endif

UDAWSequencerData* UM2SoundVertex::GetSequencerData() const
{
	return SequencerData;
}

void UM2SoundVertex::VertexNeedsBuilderUpdates()
{
	UE_LOG(unDAWDataLogs, Verbose, TEXT("Vertex needs builder updates!"))
	OnVertexNeedsBuilderUpdates.Broadcast(this);
}

void UM2SoundVertex::TransmitAudioParameter(FAudioParameter Parameter)
{
	
	if (GetSequencerData())
	{
	GetSequencerData()->OnAudioParameterFromVertex.Broadcast(Parameter);

	}
	else {
		UE_LOG(unDAWDataLogs, Error, TEXT("Outer is not sequencer data FFS!"))
	
	}
}

#if WITH_EDITOR
void UM2SoundPatch::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	
	// We don't want to rely on the PostEditChangeProperty at all unless for testing and debugging, it is not available in Shipping builds
	
	//bool bIsDirty = false;
	//FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	//if (PropertyName == TEXT("Patch"))
	//{
	//	bool bPatchImplementsInstrumentInterface = false;
	//	bool bPatchImplementsInsertInterface = false;

	//	auto interface = unDAW::Metasounds::FunDAWInstrumentRendererInterface::GetInterface();

	//	const FMetasoundFrontendVersion Version{ interface->GetName(), { interface->GetVersion().Major, interface->GetVersion().Minor } };

	//	if (Patch)
	//	{
	//		bPatchImplementsInstrumentInterface = Patch->GetDocumentChecked().Interfaces.Contains(Version);
	//	}

	//	if (bPatchImplementsInstrumentInterface)
	//	{
	//		UE_LOG(unDAWDataLogs, Log, TEXT("Patch implements the Instrument Renderer Interface!"))
	//		bHasErrors = false;
	//		VertexErrors = "None!";
	//	}
	//	else {
	//		bHasErrors = true;
	//		//ErrorType = EER
	//		VertexErrors = "Patch does not implement the Instrument Renderer Interface!";
	//	}
	//	//PatchName = PatchName;
	//	VertexNeedsBuilderUpdates();
	//	OnVertexUpdated.Broadcast();
	//}

	//UE_LOG(unDAWDataLogs, Verbose, TEXT("Property Changed %s"), *PropertyName.ToString())
}


void UM2SoundAudioInsert::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bool bIsDirty = false;
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == TEXT("Insert"))
	{
		bool bInsertImplementsInsertInterface = false;

		auto interface = unDAW::Metasounds::FunDAWCustomInsertInterface::GetInterface();

		const FMetasoundFrontendVersion Version{ interface->GetName(), { interface->GetVersion().Major, interface->GetVersion().Minor } };

		if (Patch)
		{
			bInsertImplementsInsertInterface = Patch->GetDocumentChecked().Interfaces.Contains(Version);

		}

		if (bInsertImplementsInsertInterface)
		{
			UE_LOG(unDAWDataLogs, Log, TEXT("Insert implements the Audio Insert Interface!"))
			bHasErrors = false;
			VertexErrors = "None!";
		}
		else {
			bHasErrors = true;
			//ErrorType = EER
			VertexErrors = "Insert does not implement the Audio Insert Interface!";
		}
		//PatchName = PatchName;
		VertexNeedsBuilderUpdates();
		OnVertexUpdated.Broadcast();
	}

	//UE_LOG(unDAWDataLogs, Verbose, TEXT("Property Changed %s"), *PropertyName.ToString())
}
#endif