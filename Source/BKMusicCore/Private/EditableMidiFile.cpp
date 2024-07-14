// Fill out your copyright notice in the Description page of Project Settings.

#include "EditableMidiFile.h"

void UEditableMidiFile::LoadFromHarmonixBaseFile(UMidiFile* BaseFile)
{
	UE_LOG(LogTemp, Warning, TEXT("UEditableMidiFile::LoadFromHarmonixBaseFile"));
	// Copy the base file's data
	Audio::FProxyDataInitParams InitParams{ TEXT("EditableMidiFile") };
	//InitParams .NameOfFeatureRequestingProxyData = ;
	auto ProxyData = BaseFile->CreateProxyData(InitParams);
	TheMidiData = *StaticCastSharedPtr<FMidiFileProxy>(ProxyData)->GetMidiFile();
	//TheMidiData.SongMaps.Init(TheMidiData.TicksPerQuarterNote);

	//print the track names

	for (auto& Track : TheMidiData.Tracks)
	{
		FString TrackName = *Track.GetName();
		UE_LOG(LogTemp, Warning, TEXT("Track Name: %s"), *TrackName);
	}

	RenderableCopyOfMidiFileData = nullptr;
	ProxyData.Reset();
}

void UEditableMidiFile::FinishRebuildingMidiFile()
{
	//hack for now we set the loop to 4
	//GetSongMaps()->SetLengthTotalBars(4);
	// we are now "dirty"... so make sure any new requests for renderable data
	// get a new copy...
	RenderableCopyOfMidiFileData = nullptr;
}