// Fill out your copyright notice in the Description page of Project Settings.


#include "SequencerData.h"

void UDAWSequencerData::CalculateSequenceDuration()
{
	if (!TimeStampedMidis.IsEmpty())
	{
		SequenceDuration = TimeStampedMidis[0].MidiFile->GetSongMaps()->GetSongLengthMs();
	}
}