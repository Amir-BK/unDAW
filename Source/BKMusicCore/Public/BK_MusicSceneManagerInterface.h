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
#include "MetasoundBuilderHelperBase.h"

#include "TrackPlaybackAndDisplayOptions.h"
#include "BK_MusicSceneManagerInterface.generated.h"

//forward declaration



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
	NotReady,
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

UINTERFACE(MinimalAPI, NotBlueprintable, BlueprintType, Category = "unDAW|Music Scene Manager")
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

	EBKPlayState PlayState = EBKPlayState::NotReady;

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** Please add a variable description */


	//TObjectPtr<UDAWSequencerData> SequenceData
	
	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport")
	virtual const EBKPlayState GetCurrentPlaybackState() {
		return PlayState;
	}

	UFUNCTION(Category = "BK Music")
	virtual void Entry_Initializations() {};


	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport")
	virtual void SetPlaybackState(EBKPlayState newPlayState) {};

	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport")
	virtual void SendTransportCommand(EBKTransportCommands InCommand);

	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport")
	virtual void SendSeekCommand(float InSeek) {};

	virtual FOnPlaybackStateChanged* GetPlaybackStateDelegate() = 0;

	virtual FOnTransportSeekCommand* GetSeekCommandDelegate() = 0;

	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport")
	virtual UDAWSequencerData* GetActiveSessionData() = 0;

	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport")
	virtual UAudioComponent* GetAudioComponent() = 0;

	UFUNCTION(BlueprintCallable, Category = "unDAW|Scene Manager")
	virtual TSubclassOf<UMetasoundBuilderHelperBase> GetBuilderBPClass() = 0;

	
	//for internal non BP calls to actually setup the metasound builder system
	UMetasoundBuilderHelperBase* InitializeAudioBlock();

	
	
};
