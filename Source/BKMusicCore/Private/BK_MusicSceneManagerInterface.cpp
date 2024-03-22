// Fill out your copyright notice in the Description page of Project Settings.


#include "BK_MusicSceneManagerInterface.h"


// Add default functionality here for any IBK_MusicSceneManagerInterface functions that are not pure virtual.

void UDAWSequencerData::CalculateSequenceDuration()
{
	if (!TimeStampedMidis.IsEmpty())
	{
		SequenceDuration = TimeStampedMidis[0].MidiFile->GetSongMaps()->GetSongLengthMs();
	}
}
