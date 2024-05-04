// Fill out your copyright notice in the Description page of Project Settings.



#pragma once

#include "CoreMinimal.h"
#include "HarmonixMidi/BarMap.h"
#include "HarmonixMidi/MidiFile.h"
#include "Sound/SoundWave.h"
#include "Curves/RichCurve.h"

#include "Curves/RealCurve.h"
#include "Engine/DataAsset.h"
#include "Curves/CurveFloat.h"

#include "Metasound.h"
#include "MetasoundBuilderSubsystem.h"

#include "TrackPlaybackAndDisplayOptions.h"

#include "SequencerData.generated.h"


BKMUSICCORE_API DECLARE_LOG_CATEGORY_EXTERN(unDAWDataLogs, Verbose, All);


USTRUCT(BlueprintType)
struct FLinkedMidiEvents
{

	GENERATED_BODY()

	FLinkedMidiEvents(const FMidiEvent& StartEvent, const FMidiEvent& EndEvent, const int32 inStartIndex, const int32 inEndindex)
		:StartIndex(inStartIndex),
		EndIndex(inEndindex)

	{
		StartTick = StartEvent.GetTick();
		EndTick = EndEvent.GetTick();
		pitch = StartEvent.GetMsg().Data1;
	}

	FLinkedMidiEvents()
	{

	}

	//
	//FMidiEvent StartEvent;
	//FMidiEvent EndEvent;
	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 StartIndex = 0;


	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 EndIndex = 0;


	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	uint8 pitch = 0;


	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 StartTick = 0;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 EndTick = 0;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 TrackID = -1;
	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	double Duration = 0.0;
	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	double StartTime = 0.0;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	float cornerRadius = 0.0f;

	void CalculateDuration(FSongMaps* SongsMap)
	{
		StartTime = SongsMap->TickToMs(StartTick);
		Duration = SongsMap->TickToMs(EndTick) - StartTime;
		
	}
};

USTRUCT(BlueprintType)
struct FLinkedNotesTrack
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<FLinkedMidiEvents> LinkedNotes;
};


USTRUCT(BlueprintType)
struct FBPMidiStruct {

GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="unDAW|Midi|Note Data")
	int StartTick = 0;
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Midi|Note Data")
	int EndTick = 0;
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Midi|Note Data")
	int Duration = 0;
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Midi|Note Data")
	int Pitch = 0;
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Midi|Note Data")
	int Velocity = 0;
};



//Helper object used by the pianoroll and other visualizers to easily query the end time of a playing note and the next note on a give track/pitch
UCLASS(BlueprintType)
class BKMUSICCORE_API UParsedMidiTrack : public UObject
{
	
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Midi Data")
	int TrackUniqueIndex = -1;

	
	//This can't be a UPROPERTY, we need to expose functions to interact with this data
	TArray<FLinkedMidiEvents> TrackData;


};

/**
 * 
 */
class BKMUSICCORE_API SequencerData
{
	
public:
	
};


USTRUCT(BlueprintType, Category = "unDAW|Music Scene Manager")
struct FMasterChannelOutputSettings {
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, Category = "unDAW|Music Scene Manager", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float MasterVolume = 1.0f;


	UPROPERTY(EditAnywhere, Category = "unDAW|Music Scene Manager", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float ClickVolume = 1.0f;



	UPROPERTY(EditAnywhere, Category = "unDAW|Music Scene Manager")
	bool bClickActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW|Music Scene Manager")
	EMetaSoundOutputAudioFormat OutputFormat = EMetaSoundOutputAudioFormat::Stereo;



};


USTRUCT(BlueprintType, Category = "unDAW|Music Scene Manager")
struct FTimeStamppedWavContainer {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "unDAW|Music Scene Manager")
	FMusicTimestamp TimeStamp;

	UPROPERTY(EditAnywhere, Category = "unDAW|Music Scene Manager")
	TObjectPtr <USoundWave> SoundWave;

};

USTRUCT(BlueprintType, Category = "unDAW|Music Scene Manager")
struct FTimeStamppedCurveContainer {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager")
	FMusicTimestamp TimeStamp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager")
	TObjectPtr <UCurveFloat> Curve;

};

USTRUCT(BlueprintType, Category = "unDAW|Music Scene Manager")
struct FTimeStamppedMidiContainer {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager")
	FMusicTimestamp TimeStamp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager")
	TObjectPtr<UMidiFile> MidiFile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager")
	bool bIsClockSource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW|Music Scene Manager", meta = (TitleProperty = "trackName"))
	TArray<FTrackDisplayOptions> TracksMappings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager")
	TMap<int, UParsedMidiTrack*> TrackMidiDataMap;

};

UCLASS(BlueprintType, EditInlineNew, Category = "unDAW|Music Scene Manager")
class BKMUSICCORE_API UDAWSequencerData : public UObject
{
	GENERATED_BODY()
public:
	
	FTrackDisplayOptions InvalidTrackRef;
	
	UPROPERTY(VisibleAnywhere, Category = "unDAW|Music Scene Manager")
	TMap<int, FTrackDisplayOptions> TrackDisplayOptionsMap;

	void InitTracksFromFoundArray(TMap<int, int> InTracks) {
		TrackDisplayOptionsMap.Empty();
		for (const auto& [trackID, channelID] : InTracks)
		{
			FTrackDisplayOptions newTrack;
			newTrack.ChannelIndexInParentMidi = channelID;
			newTrack.trackName = *HarmonixMidiFile->GetTrack(trackID - 1)->GetName();
			newTrack.trackColor = FLinearColor::MakeRandomSeededColor(channelID);
			TrackDisplayOptionsMap.Add(channelID, newTrack);
		}

	};

	virtual FTrackDisplayOptions& GetTracksDisplayOptions(int ID)
	{
		if (TrackDisplayOptionsMap.Contains(ID))
		{
			return TrackDisplayOptionsMap[ID];
		}
		else
		{
			return InvalidTrackRef;
		}
	};

	FTrackDisplayOptions& GetTrackOptionsRef(int TrackID)
	{
		if (TrackDisplayOptionsMap.Contains(TrackID))
		{
			return TrackDisplayOptionsMap[TrackID];
		}
		else
		{
			return InvalidTrackRef;
		}
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW|Defaults", meta = (ExposeOnSpawn = true))
	TObjectPtr<UFusionPatch> DefaultFusionPatch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW|Music Scene Manager|Meta Sound")
	TObjectPtr<UMetaSoundSource> PerformanceMetaSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager")
	float SequenceDuration = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Music Scene Manager")
	float TransportPosition = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager", meta = (ShowInnerProperties = "true", DisplayPriority = "0", ExposeOnSpawn = "true", EditInLine = "true"))
	FMasterChannelOutputSettings MasterOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager", meta = (ShowInnerProperties = "true", DisplayPriority = "2", ExposeOnSpawn = "true", EditInLine = "true"))
	TArray<FTimeStamppedCurveContainer> TimeStampedCurves;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager", meta = (ShowInnerProperties = "true", DisplayPriority = "3", ExposeOnSpawn = "true", EditInLine = "true"))
	TArray<FTimeStamppedWavContainer> TimeStampedWavs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager", meta = (TitleProperty = "MidiFile", ShowInnerProperties = "true", DisplayPriority = "1", ExposeOnSpawn = "true", EditInLine = "true"))
	TArray<FTimeStamppedMidiContainer> TimeStampedMidis;

	UFUNCTION(CallInEditor, Category = "unDAW")
	void CalculateSequenceDuration();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "unDAW")
	virtual TArray<FBPMidiStruct> GetMidiDataForTrack(const int trackID);

	TMap<int, FLinkedMidiEvents> LinkedMidiNotesMap;

	UFUNCTION(BlueprintCallable,BlueprintPure)
	static bool IsFloatNearlyZero(UPARAM(ref) const float& value, UPARAM(ref) const float& tolerance);

	UFUNCTION()
	void PopulateFromMidiFile(UMidiFile* inMidiFile);

	UPROPERTY()
	UMidiFile* HarmonixMidiFile;

	UPROPERTY()
	TArray<FMidiEvent> TempoEvents;

	UPROPERTY()
	TArray<FMidiEvent> TimeSignatureEvents;

	//this is a map that sorts the midi events by track and links start/end events with each other, needed for the pianoroll and other visualizers

	UPROPERTY()
	TMap<int, FLinkedNotesTrack> LinkedNoteDataMap;


};

