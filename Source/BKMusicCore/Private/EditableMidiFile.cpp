// Fill out your copyright notice in the Description page of Project Settings.

#include "EditableMidiFile.h"
#include "Curves/CurveFloat.h"

void UEditableMidiFile::LoadFromHarmonixBaseFile(UMidiFile* BaseFile, UCurveFloat* InTempoCurve, int32 InOffsetTicks)
{
	UE_LOG(LogTemp, Warning, TEXT("UEditableMidiFile::LoadFromHarmonixBaseFile"));
	// Copy the base file's data
	Audio::FProxyDataInitParams InitParams{ TEXT("EditableMidiFile") };
	//InitParams .NameOfFeatureRequestingProxyData = ;
	auto ProxyData = BaseFile->CreateProxyData(InitParams);
	TheMidiData = *StaticCastSharedPtr<FMidiFileProxy>(ProxyData)->GetMidiFile();
	
	if(InOffsetTicks != 0)
	{
		int i = 0;
		for (auto& Track : TheMidiData.Tracks)
		{
			//skip the first track as its the tempo track, the use case I support is to offset all tracks except the tempo track
			//if (i++ == 0) continue;

			//new midi track
			auto NewTrack = FMidiTrack(*Track.GetName());

			for (auto& Event : Track.GetEvents())
			{
				//Track.ChangeTick(Track.GetRawEvents(), Event.GetTick() + InOffsetTicks);
				//if event is text and tick == 0, skip
				if(Event.GetMsg().MsgType() == FMidiMsg::EType::Text && Event.GetTick() == 0)
				{
					//NewTrack.AddEvent(Event);
					continue;
				}

				//if event is tempo or time sig event and tick is 0 skip
				if(Event.GetMsg().MsgType() == FMidiMsg::EType::Tempo || Event.GetMsg().MsgType() == FMidiMsg::EType::TimeSig)
				{
					if(Event.GetTick() == 0)
					{
						NewTrack.AddEvent(Event);
						continue;
					}
				}

				auto NewMessage = Event.GetMsg();
				FMidiEvent NewEvent = FMidiEvent(Event.GetTick() + InOffsetTicks, Event.GetMsg());
				NewTrack.AddEvent(NewEvent);
			}

			TheMidiData.Tracks[i++] = NewTrack;
		}
		SortAllTracks();
	}

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
	//PopulateTempoCurve();
	//OnTempoCurveChanged

	SortAllTracks();

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
