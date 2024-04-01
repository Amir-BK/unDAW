// Fill out your copyright notice in the Description page of Project Settings.


#include "SequencerData.h"

DEFINE_LOG_CATEGORY(unDAWDataLogs);


struct FEventsWithIndex
{
	FMidiEvent event;
	int32 eventIndex;
};

void UDAWSequencerData::CalculateSequenceDuration()
{
	if (!TimeStampedMidis.IsEmpty())
	{
		SequenceDuration = TimeStampedMidis[0].MidiFile->GetSongMaps()->GetSongLengthMs();
	}
}

TArray<FBPMidiStruct> UDAWSequencerData::GetMidiDataForTrack(const int trackID)
{
	return TArray<FBPMidiStruct>();
}

bool UDAWSequencerData::IsFloatNearlyZero(UPARAM(ref) const float& value, UPARAM(ref) const float& tolerance)
{
	
	return FMath::IsNearlyZero(value, tolerance);
}

void UDAWSequencerData::PopulateFromMidiFile(UMidiFile* inMidiFile)
{
	
	int numTracks = 0;
	int numTracksRaw = 0;
	
	for (auto& track : inMidiFile->GetTracks())
	{
		//if track has no events we can continue, but this never happens, it might not have note events but it has events.
		int numTracksInternal = numTracksRaw++;
		if (track.GetEvents().IsEmpty()) continue;

		TMap<int, TArray<FLinkedMidiEvents*>> channelsMap;



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

							FLinkedMidiEvents* foundPair = new FLinkedMidiEvents(unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].event, MidiEvent,
								unlinkedNotesIndexed[MidiEvent.GetMsg().GetStdData1()].eventIndex, index);
							linkedNotes.Add(foundPair);
							// sort the tracks into channels
							if (channelsMap.Contains(midiChannel))
							{
								channelsMap[midiChannel].Add(foundPair);
							}
							else {
								channelsMap.Add(TTuple<int, TArray<FLinkedMidiEvents*>>(midiChannel, TArray<FLinkedMidiEvents*>()));
								channelsMap[midiChannel].Add(foundPair);
							}

						}

					}
				};

				break;
			case FMidiMsg::EType::Tempo:
				UE_LOG(unDAWDataLogs, Verbose, TEXT("We receive a tempo event! data1 %d data2 %d"), MidiEvent.GetMsg().Data1, MidiEvent.GetMsg().Data2)
					//MidiEvent.GetMsg().Data1
					TempoEvents.Add(MidiEvent);
				break;
			case FMidiMsg::EType::TimeSig:
				UE_LOG(unDAWDataLogs, Verbose, TEXT("We receive a time signature event!"))
					TimeSignatureEvents.Add(MidiEvent);
				break;
			case FMidiMsg::EType::Text:
				UE_LOG(unDAWDataLogs, Verbose, TEXT("We receive a text event??? %s"), *MidiEvent.GetMsg().ToString(MidiEvent.GetMsg()))
					break;
			case FMidiMsg::EType::Runtime:
				UE_LOG(unDAWDataLogs, Verbose, TEXT("We receive a runtime event???"))
					break;
			}

			++index;

		}
		// if we couldn't find any linked notes this track is a control track, contains no notes.

		if (channelsMap.IsEmpty()) continue;

		//init data asset MIDI if not init

		UE_LOG(unDAWDataLogs, Log, TEXT("Num Channel Buckets: %d"), channelsMap.Num())

			for (auto& [channel, notes] : channelsMap)
			{
				
				//this is the init code for the display options, should be moved to it's own functions
				bool hasDataForTrack = false;

				if (TimeStampedMidis[0].TracksMappings.IsValidIndex(numTracks))
				{
					//newTrackDisplayOptions = TrackCache->TimeStampedMidis[0].TracksMappings[numTracks];
					//tracksDisplayOptions.Add(TrackCache->TimeStampedMidis[0].TracksMappings[numTracks]);
					hasDataForTrack = true;

				}
				if (!hasDataForTrack) {
					FTrackDisplayOptions newTrackDisplayOptions = FTrackDisplayOptions();
					newTrackDisplayOptions.fusionPatch = DefaultFusionPatch;
					newTrackDisplayOptions.TrackIndexInParentMidi = numTracksInternal;
					newTrackDisplayOptions.ChannelIndexInParentMidi = channelsMap.Num() == 1 ? 0 : channel;
					newTrackDisplayOptions.trackColor = FLinearColor::MakeRandomSeededColor((numTracksInternal + 1) * (channel + 1));
					newTrackDisplayOptions.trackName = *track.GetName();
					TimeStampedMidis[0].TracksMappings.Add(newTrackDisplayOptions);
				}



				//if (!notes.IsEmpty())
				//{
				//	for (const auto& foundPair : notes)
				//	{
				//		PianoRollGraph->AddNote(*foundPair, numTracks, numTracksInternal);
				//	}
				//}

				numTracks++;

			}

	};

}
