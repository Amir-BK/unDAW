// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicDeviceControllerSubsystem.h"

namespace MusicDeviceRegistry
{

	static TMap<FString, UMIDIDeviceInputController*> MidiDeviceControllers;

}

UMIDIDeviceInputController* UMusicDeviceControllerSubsystem::GetOrCreateMidiInputDeviceController(const FString& MidiDeviceName)
{
	//if the registry already contains device name, return it from registry, otherwise create a new one and add it to the registry
	if (MusicDeviceRegistry::MidiDeviceControllers.Contains(MidiDeviceName))
	{
		return MusicDeviceRegistry::MidiDeviceControllers[MidiDeviceName];
	}
	else
	{
		TArray<FMIDIDeviceInfo> InputDevices, OutputDevices;
		UMIDIDeviceManager::FindAllMIDIDeviceInfo(InputDevices, OutputDevices);

		//Find input device with the device name
		FMIDIDeviceInfo* NewDeviceToCreate = InputDevices.FindByPredicate([MidiDeviceName](const FMIDIDeviceInfo& DeviceInfo)
			{
				return DeviceInfo.DeviceName.Contains(MidiDeviceName);
			});

		if (NewDeviceToCreate == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find MIDI device with name %s"), *MidiDeviceName);
			return nullptr;
		}
		
		UMIDIDeviceInputController* NewController = UMIDIDeviceManager::CreateMIDIDeviceInputController(NewDeviceToCreate->DeviceID, 512);
	
		MusicDeviceRegistry::MidiDeviceControllers.Add(MidiDeviceName, NewController);
		return NewController;
	}
}
