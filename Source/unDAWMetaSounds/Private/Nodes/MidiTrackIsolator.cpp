// Copyright Epic Games, Inc. All Rights Reserved.

#include "MidiTrackIsolator.h"

namespace unDAWMetasounds::TrackIsolatorOP
{
	void FMidiTrackIsolator::Process(const HarmonixMetasound::FMidiStream& InStream, HarmonixMetasound::FMidiStream& OutStream)
	{
		const auto Filter = [this](const HarmonixMetasound::FMidiStreamEvent& Event)
		{
			ensure(Event.TrackIndex >= 0 && Event.TrackIndex <= UINT16_MAX);
			const uint16 TrackIndex = static_cast<uint16>(Event.TrackIndex);
			bool IncludeEvent = false;

			if (TrackIndex == 0)
			{
				IncludeEvent = IncludeConductorTrack;
			}
			else if (TrackIndex >= TrackIdx && TrackIndex <= ChannelIdx)
			{
				IncludeEvent = true;
			}

			//if(IncludeEvent) Event.TrackIndex = 1;

			return IncludeEvent;
		};


		for (auto& Event : InStream.GetEventsInBlock())

		{
			if (!Event.MidiMessage.IsStd()) continue;
			//if (!Event.MidiMessage.IsNoteMessage()) continue;

			const uint16 TrackIndex = static_cast<uint16>(Event.TrackIndex);
			const uint16 ChannelIndex = static_cast<uint16>(Event.MidiMessage.GetStdChannel());
			
			bool IncludeEvent = false;
			if (TrackIndex == TrackIdx)
			{
				if (ChannelIndex == ChannelIdx)
				{
					IncludeEvent = true;
				}
			}

			//UE_LOG(LogTemp, Warning, TEXT("TrackIndex: %d, ChannelIndex: %d. Filter Track Index Min: %d, Max: %d"), TrackIndex, ChannelIndex, TrackIdx, ChannelIdx);

			if (IncludeEvent)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Found Note!"));
				auto NonConst = const_cast<HarmonixMetasound::FMidiStreamEvent&>(Event);
				NonConst.TrackIndex = 1;
				OutStream.AddMidiEvent(NonConst);
			}

			

		}
		

		//HarmonixMetasound::FMidiStream::Copy(InStream, OutStream, Filter);



		// Unstick notes if necessary
		StuckNoteGuard.Process(InStream, OutStream, Filter);
	}

	void FMidiTrackIsolator::SetFilterValues(uint16 InTrackIndex, uint16 InChannelIndex, bool InIncludeConductorTrack)
	{
		TrackIdx = InTrackIndex;
		ChannelIdx = InChannelIndex;
		IncludeConductorTrack = false;
	}
}
