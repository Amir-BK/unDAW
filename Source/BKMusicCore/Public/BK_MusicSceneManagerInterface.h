// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HarmonixMidi/BarMap.h"
#include "HarmonixMidi/MidiFile.h"
#include "Sound/SoundWave.h"
#include "Curves/RichCurve.h"
#include "Components/AudioComponent.h"

#include "Curves/RealCurve.h"
#include "Engine/DataAsset.h"
#include "Curves/CurveFloat.h"

#include "Metasound.h"
#include "MetasoundBuilderSubsystem.h"
#include "MetasoundGeneratorHandle.h"
#include "M2SoundGraphData.h"
#include "M2SoundGraphRenderer.h"

#include "TrackPlaybackAndDisplayOptions.h"
#include "BK_MusicSceneManagerInterface.generated.h"



BKMUSICCORE_API DECLARE_LOG_CATEGORY_EXTERN(BKMusicInterfaceLogs, Verbose, All);


class UM2SoundGraphRenderer;




DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransportCommand, EBKTransportCommands, NewCommand);



// This class does not need to be modified.



UINTERFACE(MinimalAPI, NotBlueprintable, BlueprintType, Category = "unDAW Sequence")
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

	TAttribute<FMusicTimestamp> CurrentTimestamp;

	
	FOnTransportSeekCommand OnSeekEvent;


	EBKPlayState PlayState = EBKPlayState::NotReady;

	TObjectPtr<UDAWSequencerData> SequenceData;

	//as the music scene manager is a data container the core playback functionality is managed by the performer,
	//this is true both for in scene usages as well as for the asset toolkit preview/editing in editor
	UM2SoundGraphRenderer* Performer;

	void CreatePerformer(UAudioComponent* InAudioComponent);
	
	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport")
	virtual const EBKPlayState GetCurrentPlaybackState();

	virtual void Entry_Initializations() {};

	virtual const UM2SoundGraphRenderer* GetPerformer() { return Performer; };

	virtual UDAWSequencerData* GetDAWSequencerData() const;


	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport")
	virtual void SetPlaybackState(EBKPlayState newPlayState) { PlayState = newPlayState; };

	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport", CallInEditor)
	virtual void SendTransportCommand(EBKTransportCommands InCommand);

	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport", CallInEditor)
	virtual void SetPlayrate(float newPlayrate);

	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport", CallInEditor)
	virtual const float GetPlayrate() { return Playrate; };

	float Playrate = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport", CallInEditor)
	virtual void SendSeekCommand(float InSeek);



	FOnCreateAuditionGeneratorHandleDelegate GeneratorCreated;

};
