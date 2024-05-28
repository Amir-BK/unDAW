// Fill out your copyright notice in the Description page of Project Settings.


#include "SequencerData.h"
#include "UnDAWSequencePerformer.h"

DEFINE_LOG_CATEGORY(unDAWDataLogs);


struct FEventsWithIndex
{
	FMidiEvent event;
	int32 eventIndex;
};


	

void UDAWSequencerData::AddVertex(UM2SoundVertex* Vertex)
{
	if(auto NewOutput = Cast<UM2SoundOutput>(Vertex))
	{
		Outputs.Add(NewOutput->GetFName(), NewOutput);
	}
}

void UDAWSequencerData::ChangeFusionPatchInTrack(int TrackID, UFusionPatch* NewPatch)
{
	if (TrackDisplayOptionsMap.Contains(TrackID))
	{
		TrackDisplayOptionsMap[TrackID].fusionPatch = NewPatch;
		OnFusionPatchChangedInTrack.ExecuteIfBound(TrackID, NewPatch);
	}
}

inline void UDAWSequencerData::InitTracksFromFoundArray(TMap<int, int> InTracks) {

	auto PianoPatchPath = FSoftObjectPath(TEXT("/Harmonix/Examples/Patches/Piano.Piano"));

	UFusionPatch* PianoPatch = static_cast<UFusionPatch*>(PianoPatchPath.TryLoad());
	TrackDisplayOptionsMap.Empty();
	for (const auto& [trackID, channelID] : InTracks)
	{

		FTrackDisplayOptions newTrack;
		newTrack.ChannelIndexInParentMidi = channelID;

		//FString::AppendInt(channelID, newTrack.trackName);
		newTrack.trackName = *HarmonixMidiFile->GetTrack(trackID - 1)->GetName() + " Ch: " + FString::FromInt(channelID) + " Tr: " + FString::FromInt(trackID - 1);
		newTrack.trackColor = FLinearColor::MakeRandomSeededColor(channelID);
		newTrack.fusionPatch = PianoPatch;
		TrackDisplayOptionsMap.Add(channelID, newTrack);
		Outputs.Add(FName(newTrack.trackName), NewObject<UM2SoundOutput>(this, NAME_None, RF_Transactional));
		TrackInputs.Add(trackID, NewObject<UM2SoundTrackInput>(this, NAME_None, RF_Transactional));
	}

}

inline FTrackDisplayOptions& UDAWSequencerData::GetTracksDisplayOptions(int ID)
{
	if (TrackDisplayOptionsMap.Contains(ID))
	{
		return TrackDisplayOptionsMap[ID];
	}
	else
	{
		return InvalidTrackRef;
	}
}

inline FTrackDisplayOptions& UDAWSequencerData::GetTrackOptionsRef(int TrackID)
{
	if (TrackDisplayOptionsMap.Contains(TrackID))
	{
		return TrackDisplayOptionsMap[TrackID];
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



void UDAWSequencerData::PopulateFromMidiFile(UMidiFile* inMidiFile)
{
	TMap<int, int> FoundChannels;
	LinkedNoteDataMap.Empty();
	Outputs.Empty();
	TrackInputs.Empty();
	HarmonixMidiFile = inMidiFile;
	//MidiSongMap = HarmonixMidiFile->GetSongMaps();

	int numTracks = 0;
	int numTracksRaw = 0;

	if (HarmonixMidiFile == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("No midi file! This is strange!"))
			return;
	}
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
						//MidiEvent.GetMsg().
						//unlinkedNotesIndexed
						if (midiChannel == unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].event.GetMsg().GetStdChannel())
						{

							FLinkedMidiEvents foundPair = FLinkedMidiEvents(unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].event, MidiEvent,
								unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].eventIndex, index);

							foundPair.TrackID = midiChannel == TrackMainChannel ? numTracksInternal : midiChannel;
							FoundChannels.Add(numTracksRaw, foundPair.TrackID);
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
		InitTracksFromFoundArray(FoundChannels);
#if WITH_EDITOR
		if(M2SoundGraph)
		{
			M2SoundGraph->InitializeGraph();
		}
#endif
		//CreateBuilderHelper();
	}
}

UDAWSequencerPerformer* UDAWSequencerData::CreatePerformer(UAudioComponent* AuditionComponent)
{
	auto SequencerPerformer = NewObject<UDAWSequencerPerformer>(this);
	SequencerPerformer->SessionData = this;
	SequencerPerformer->OutputFormat = MasterOptions.OutputFormat;
	SequencerPerformer->MidiTracks = &TrackDisplayOptionsMap;
	OnFusionPatchChangedInTrack.BindUObject(SequencerPerformer, &UDAWSequencerPerformer::ChangeFusionPatchInTrack);
	AuditionComponent->SetVolumeMultiplier(MasterOptions.MasterVolume);
	SequencerPerformer->InitBuilderHelper("unDAW Session Renderer", AuditionComponent);

	return SequencerPerformer;
}

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
#if WITH_EDITOR
		if (EditorPreviewPerformer && EditorPreviewPerformer->AuditionComponentRef)
		{
			EditorPreviewPerformer->AuditionComponentRef->SetVolumeMultiplier(MasterOptions.MasterVolume);
		}
#endif
	}

	UE_LOG(unDAWDataLogs, Verbose, TEXT("Property Changed %s"), *PropertyName.ToString())
}
