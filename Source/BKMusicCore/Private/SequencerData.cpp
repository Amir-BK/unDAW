// Fill out your copyright notice in the Description page of Project Settings.


#include "SequencerData.h"

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
