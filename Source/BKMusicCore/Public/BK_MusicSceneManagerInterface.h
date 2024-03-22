// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
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
#include "BK_MusicSceneManagerInterface.generated.h"


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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW|Music Scene Manager")
	TArray<FTrackDisplayOptions> TracksMappings;

};

UCLASS(BlueprintType, EditInlineNew, Category = "unDAW|Music Scene Manager")
class BKMUSICCORE_API UDAWSequencerData : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager", meta = (ShowInnerProperties = "true", DisplayPriority = "0", ExposeOnSpawn = "true", EditInLine = "true"))
	FMasterChannelOutputSettings MasterOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager", meta = (ShowInnerProperties = "true", DisplayPriority = "2", ExposeOnSpawn = "true", EditInLine = "true"))
	TArray<FTimeStamppedCurveContainer> TimeStampedCurves;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager", meta = (ShowInnerProperties = "true", DisplayPriority = "3", ExposeOnSpawn = "true", EditInLine = "true"))
	TArray<FTimeStamppedWavContainer> TimeStampedWavs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Music Scene Manager", meta = (ShowInnerProperties = "true", DisplayPriority = "1", ExposeOnSpawn = "true", EditInLine = "true"))
	TArray<FTimeStamppedMidiContainer> TimeStampedMidis;


};

UENUM(BlueprintType, Category = "unDAW|Music Scene Manager")
enum EBKTransportCommands : uint8
{
	Init,
	Play,
	Pause,
	Stop,
	Kill,
	TransportBackward,
	TransportForward,
	NextMarker,
	PrevMarker

};

UENUM(BlueprintType, Category = "unDAW|Music Scene Manager")
enum EBKPlayState : uint8
{
	Preparing,
	ReadyToPlay,
	Playing,
	Seeking,
	Paused

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaybackStateChanged, EBKPlayState, NewPlaystate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransportCommand, EBKTransportCommands, NewCommand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransportSeekCommand, float, NewSeekTarget);

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable, BlueprintType, Category = "unDAW|Music Scene Manager")
class UBK_MusicSceneManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BKMUSICCORE_API IBK_MusicSceneManagerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** Please add a variable description */


	
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "unDAW|Transport")
	const EBKPlayState GetCurrentPlaybackState();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "unDAW|Transport")
	void SendTransportCommand(const EBKTransportCommands InCommand);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "unDAW|Transport")
	void SendSeekCommand(const float InSeek);

	virtual FOnPlaybackStateChanged* GetPlaybackStateDelegate() = 0;

	virtual FOnTransportSeekCommand* GetSeekCommandDelegate() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "unDAW|Transport")
	UAudioComponent* GetAudioComponent();

	
	
};
