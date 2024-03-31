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




struct FLinkedMidiEvents
{


	FLinkedMidiEvents(const FMidiEvent& StartEvent, const FMidiEvent& EndEvent, const int32 inStartIndex, const int32 inEndindex)
		: StartEvent(StartEvent),
		EndEvent(EndEvent),
		StartIndex(inStartIndex),
		EndIndex(inEndindex)

	{
	}

	//GENERATED_BODY()
	FMidiEvent StartEvent;
	FMidiEvent EndEvent;
	int32 StartIndex;
	int32 EndIndex;
};


USTRUCT(BlueprintType)
struct FBPMidiStruct {

GENERATED_BODY()

	int StartTick = 0;
	int EndTick = 0;
	int Duration = 0;
	int Pitch = 0;
	int Velocity = 0;
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

};

UCLASS(BlueprintType, EditInlineNew, Category = "unDAW|Music Scene Manager")
class BKMUSICCORE_API UDAWSequencerData : public UObject
{
	GENERATED_BODY()
public:

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


};

