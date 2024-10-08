// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "HarmonixMetasound/MidiOps/StuckNoteGuard.h"

#include "HarmonixMetasound/DataTypes/MidiStream.h"

namespace unDAWMetasounds::TrackIsolatorOP
{
	/**
	 * Filters MIDI events based on which track they're on
	 */
	class UNDAWMETASOUNDS_API FMidiTrackIsolator
	{
	public:
		/**
		 * Pass the filtered events from the input stream to the output stream
		 * @param InStream The MIDI stream to filter
		 * @param OutStream The filtered MIDI stream
		 */
		void Process(const HarmonixMetasound::FMidiStream& InStream, HarmonixMetasound::FMidiStream& OutStream);

		/**
		 * Set the range of tracks to include. Track 0 is reserved as the "conductor" track, which only contains timing information.
		 * The first track with notes and other messages on it will be track 1.
		 * @param InMinTrackIdx The first track to include in the range of tracks (inclusive).
		 * @param InMaxTrackIdx The last track to include in th range of tracks (inclusive).
		 * @param InIncludeConductorTrack Whether or not to include the conductor track (track 0)
		 */
		void SetFilterValues(uint16 InTrackIndex, uint16 InChannelIndex, bool InIncludeConductorTrack);

	private:
		uint16 TrackIdx = 0;
		uint16 ChannelIdx = 0;
		bool IncludeConductorTrack = false;

		Harmonix::Midi::Ops::FStuckNoteGuard StuckNoteGuard;
	};
}

namespace unDAWMetasounds::MidiStreamEventTrackMergeOp
{
	/**
	Adds notes from a TArray<TTupe<uint32, FMidiMsg>> to a FMidiStream

	*/
	class UNDAWMETASOUNDS_API FMidiStreamEventTrackMerge
	{
	public:
		void Process(const HarmonixMetasound::FMidiStream& InStream, TArray<TTuple<int32, FMidiMsg>> InEvents, HarmonixMetasound::FMidiStream& OutStream);;

	};


};