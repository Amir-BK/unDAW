// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#ifdef WITH_MIDI_DEVICE
#include "MidiDeviceManager.h"
#endif
#include "MusicDeviceControllerSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_SixParams(FOnDAWRawMidiEvent, FName /* MusicDeviceController */, int32 /* Timestamp */, int32 /* Type */, int32 /* Channel */, int32 /* MessageData1 */, int32 /* MessageData2 */);

//cross platform safe aggregator for midi devices and other inputs
UCLASS()
class BKMUSICCORE_API UMusicDeviceInput : public UObject
{
	GENERATED_BODY()

	FOnDAWRawMidiEvent OnDAWRawMidiEvent;

#ifdef WITH_MIDI_DEVICE
	UMIDIDeviceInputController* UnderlyingMidiDeviceController;
#endif

};

/**
 * This subsystem solves some annoyances with the unreal portmidi implementation, namely it lets us keep a mididevice controller alive
 * and not have to worry about it remaining in a 'used' state when we try to invoke it again
 * but this subsystem is also a good place to put other music device controllers, like OSC, MIDI, etc.
 * perhaps we will allow allow injecting midi and custom events on these endpoints
 */
UCLASS()
class BKMUSICCORE_API UMusicDeviceControllerSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	
	//as shuttindg down midi devices doesn't really work in unreal, we will keep a registry of all the midi devices we have created
	//the user probably doesn't need to destroy them at all, they can remain in memory, otherwise they're just not gonna be accessible
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MusicDeviceControllerSubsystem")
	static UMIDIDeviceInputController* GetOrCreateMidiInputDeviceController(const FString& MidiDeviceName);

	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MusicDeviceControllerSubsystem")
	//static UMusicDeviceInput* GetOrCreateMusicDeviceInput(const FString& MusicDeviceName) {};
};
