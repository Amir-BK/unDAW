// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetasoundGeneratorHandle.h"
#include "GameFramework/Actor.h"
#include "BK_MusicSceneManagerInterface.h"
#include "Components/AudioComponent.h"
#include "HarmonixMetasound/Components/MusicClockComponent.h"
#include "HarmonixMetasound/Components/MusicTempometerComponent.h"
#include "HarmonixMidi/MidiSongPos.h"

#include "Delegates/DelegateBase.h"
#include "Delegates/DelegateSettings.h"
#include "MusicScenePlayerActor.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTriggerExecuted, FName, TriggerName);

UCLASS()
class BKMUSICCORE_API AMusicScenePlayerActor : public AActor , public IBK_MusicSceneManagerInterface
{
	GENERATED_BODY()

public:
	
	//creates a transient music timestamp to trigger node using the metasound builder, connects its output to the graph output, watches the output
	// via the metasound output subsystem and FINALLY, calls the delegate when the trigger is executed
	UFUNCTION(BlueprintCallable, Category = "unDAW|Quantization", meta=(AutoCreateRefTerm = "InDelegate", Keywords = "Event, Quantization, DAW"))
	void SubscribeToTriggerEventOnMusicTimestamp(FName TriggerName, FMusicTimestamp TriggerTime, const FOnTriggerExecuted& InDelegate) {};

	UFUNCTION(BlueprintCallable, Category = "unDAW|Quantization", meta=(AutoCreateRefTerm = "InDelegate", Keywords = "Event, Quantization, DAW"))
	void SubscribeToTriggerEventOnNextBar(FName TriggerName, FMusicTimestamp TriggerTime, const FOnTriggerExecuted& InDelegate) {};

	UFUNCTION(BlueprintCallable, Category = "unDAW|Quantization", meta=(AutoCreateRefTerm = "InDelegate", Keywords = "Event, Quantization, DAW"))
	void SubscribeToTriggerEventOnNextBeat(FName TriggerName, FMusicTimestamp TriggerTime, const FOnTriggerExecuted& InDelegate) {};

	//we'll need to test to see how this works with very fast tempos
	UFUNCTION(BlueprintCallable, Category = "unDAW|Quantization", meta=(AutoCreateRefTerm = "InDelegate", Keywords = "Event, Quantization, DAW"))
	void SubscribeToTriggerEventOnNextQuantizationBoundary(FName TriggerName, FMusicTimestamp TriggerTime, const FOnTriggerExecuted& InDelegate) {};

	// Sets default values for this actor's properties
	AMusicScenePlayerActor();

	// DELEGATES!!!

	//UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "unDAW|Transport")
	FOnTransportSeekCommand TransportSeekDelegate;

	UPROPERTY(BlueprintAssignable, Category = "unDAW|Transport")
	FOnPlaybackStateChanged PlaystateDelegate;

	UFUNCTION()
	void DAWSequencePlayStateChange(EBKPlayState NewState);



	//To get the accurate timestamp for our MIDI players it's better to read the timestamp on the DAWSequecerData
	//The harmonix music clock component exposes a lot of nice functions and writes the tempo data to the MaterialParameterCollection
	//but it has some quirks, it's bar reading might not be accurate, especially if you send seek events to the midi player
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "unDAW|Music Scene")
	UMusicClockComponent* VideoSyncedMidiClock;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "unDAW|Music Scene")
	UMusicTempometerComponent* MusicTempometer;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "unDAW|Music Scene")
	TObjectPtr<UMaterialParameterCollection> MaterialParameterCollection;

	//UPROPERTY()
	TSharedPtr<UMetasoundGeneratorHandle> GeneratorHandle;

	UPROPERTY()
	TEnumAsByte<EBKPlayState> PlayState = EBKPlayState::NotReady;

	UPROPERTY()
	float PlaybackCursorPosition = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "unDAW", meta = (DisplayPriority = "0"))
	TObjectPtr<UDAWSequencerData> SessionData;

	//hmmm
	UDAWSequencerData* GetDAWSequencerData() const override;

	//METHODS

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void PerformanceMetasoundGeneratorCreated(TSharedPtr<Metasound::FMetasoundGenerator> GeneratorPointer, ESPMode UserPolicy);

	void PerformanceMetasoundGeneratorDestroyed(uint64 GeneratorPointer);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//creates tempometer and music clock components
	UFUNCTION()
	virtual void InitHarmonixComponents();



	UFUNCTION(BlueprintCallable, Category = "BK Music")
	void UpdateWatchers();

};
