// Fill out your copyright notice in the Description page of Project Settings.

#include "EditableMidiFile.h"
#include "Curves/CurveFloat.h"

void UEditableMidiFile::LoadFromHarmonixBaseFile(UMidiFile* BaseFile, UCurveFloat* InTempoCurve)
{
	UE_LOG(LogTemp, Warning, TEXT("UEditableMidiFile::LoadFromHarmonixBaseFile"));
	// Copy the base file's data
	Audio::FProxyDataInitParams InitParams{ TEXT("EditableMidiFile") };
	//InitParams .NameOfFeatureRequestingProxyData = ;
	auto ProxyData = BaseFile->CreateProxyData(InitParams);
	TheMidiData = *StaticCastSharedPtr<FMidiFileProxy>(ProxyData)->GetMidiFile();
	//TheMidiData.SongMaps.Init(TheMidiData.TicksPerQuarterNote);
	if(InTempoCurve)
	{
		Curve = InTempoCurve;
	}
	else
	{
		Curve = NewObject<UCurveFloat>(GetOuter());
		PopulateTempoCurve();
	}

	Curve->OnUpdateCurve.AddUObject(this, &UEditableMidiFile::OnTempoCurveChanged);
	//TheMidiData.SongMaps.GetTempoMap().AddTempoInfoPoint(Harmonix::Midi::Constants::BPMToMidiTempo(75), 0);

	//print the track names

	for (auto& Track : TheMidiData.Tracks)
	{
		FString TrackName = *Track.GetName();
		UE_LOG(LogTemp, Warning, TEXT("Track Name: %s"), *TrackName);
	}

	RenderableCopyOfMidiFileData = nullptr;
	ProxyData.Reset();
}

void UEditableMidiFile::PopulateTempoCurve()
{
	//first just get the points?
	TempoInfoPoints = TheMidiData.SongMaps.GetTempoMap().GetTempoPoints();

	for(auto& Point : TempoInfoPoints)
	{
		Curve->FloatCurve.AddKey(Point.StartTick, Point.GetBPM());
	}

}

void UEditableMidiFile::FinishRebuildingMidiFile()
{
	//hack for now we set the loop to 4
	//GetSongMaps()->SetLengthTotalBars(4);
	// we are now "dirty"... so make sure any new requests for renderable data
	// get a new copy...
	RenderableCopyOfMidiFileData = nullptr;
}

void UEditableMidiFile::OnTempoCurveChanged(UCurveBase* InCurve, EPropertyChangeType::Type Type)
{
	UE_LOG(LogTemp, Warning, TEXT("UEditableMidiFile::OnTempoCurveChanged"));
	if (Type == EPropertyChangeType::Unspecified)
	{
		// we don't care about this
		return;
	}


		// we need to update the tempo map
		// first clear it
		TheMidiData.SongMaps.GetTempoMap().Empty();
		// now add the points
		for (auto& Key : Curve->FloatCurve.Keys)
		{
			TheMidiData.SongMaps.GetTempoMap().AddTempoInfoPoint(Harmonix::Midi::Constants::BPMToMidiTempo(Key.Value), Key.Time);
		}
	
		TheMidiData.SongMaps.GetTempoMap().Finalize(TheMidiData.GetLastEventTick());
		RenderableCopyOfMidiFileData = nullptr;
}
