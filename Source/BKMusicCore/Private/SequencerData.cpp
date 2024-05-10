// Fill out your copyright notice in the Description page of Project Settings.


#include "SequencerData.h"
#include "UnDAWSequencePerformer.h"

DEFINE_LOG_CATEGORY(unDAWDataLogs);


struct FEventsWithIndex
{
	FMidiEvent event;
	int32 eventIndex;
};


	

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
		//CreateBuilderHelper();
	}
}

void UDAWSequencerData::CreateBuilderHelper(UAudioComponent* AuditionComponent)
{
	MetasoundBuilderHelper = NewObject<UDAWSequencerPerformer>(this);
	MetasoundBuilderHelper->SessionData = this;
	MetasoundBuilderHelper->OutputFormat = MasterOptions.OutputFormat;
	MetasoundBuilderHelper->MidiTracks = TrackDisplayOptionsMap;
	MetasoundBuilderHelper->InitBuilderHelper("unDAW Session Renderer", AuditionComponent);
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

	UE_LOG(unDAWDataLogs, Verbose, TEXT("Property Changed %s"), *PropertyName.ToString())
}
