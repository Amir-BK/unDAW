// Fill out your copyright notice in the Description page of Project Settings.

#include "BK_MusicSceneManagerInterface.h"

// Add default functionality here for any IBK_MusicSceneManagerInterface functions that are not pure virtual.

BKMUSICCORE_API DEFINE_LOG_CATEGORY(BKMusicInterfaceLogs);

void IBK_MusicSceneManagerInterface::CreatePerformer(UAudioComponent* InAudioComponent)
{
}

const EBKPlayState IBK_MusicSceneManagerInterface::GetCurrentPlaybackState()
{
	{
		if (!GetDAWSequencerData()) return EBKPlayState::NoBuilder;

		return GetDAWSequencerData()->PlayState;
	}
}

inline UDAWSequencerData* IBK_MusicSceneManagerInterface::GetDAWSequencerData() const {
	return SequenceData;
}

//void IBK_MusicSceneManagerInterface::SendTransportCommand(EBKTransportCommands InCommand)
//{
//	//TODODOTOD	TODTO
//
//	UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("%s (%s): ReceivedCommand: %s, "), *this->_getUObject()->GetFullName(), this->_getUObject()->GetWorld() ? *this->_getUObject()->GetWorld()->GetName() : TEXT("No world"), *UEnum::GetValueAsString(InCommand))
//	UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("Received transport Command, Current Playback State %s"), *UEnum::GetValueAsString(GetCurrentPlaybackState()))
//
//}

void IBK_MusicSceneManagerInterface::SendTransportCommand(EBKTransportCommands InCommand)
{
	{
		UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("Received transport Command, Current Playback State %s"), *UEnum::GetValueAsString(GetCurrentPlaybackState()))
			if (!GetDAWSequencerData()) return;

		GetDAWSequencerData()->SendTransportCommand(InCommand);
		//if (Performer) Performer->SendTransportCommand(InCommand);
	}
}

void IBK_MusicSceneManagerInterface::SetPlayrate(float newPlayrate)
{
	//GetAudioComponent()->SetFloatParameter(FName("unDAW.Midi.Speed"), newPlayrate);
	Playrate = newPlayrate;
}

void IBK_MusicSceneManagerInterface::SendSeekCommand(float InSeek)
{
	if (GetDAWSequencerData())
		GetDAWSequencerData()->SendSeekCommand(InSeek);

	UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("Received Seek Command"))
}

TArray<FString> IBK_MusicSceneManagerInterface::GetMidiOutputNames()
{
	//make dummy array, populate an array of FStrings with musical instrument names for co-pilot

	if (!GetDAWSequencerData()) return TArray<FString>();

	TArray<FString> MidiOutputNames;
	for (const auto& TrackMetaData : GetDAWSequencerData()->M2TrackMetadata)
	{
		MidiOutputNames.Add(TrackMetaData.TrackName);
	}

	//for (const auto& [Name, Input] : GetDAWSequencerData()->CoreNodes.MemberInputMap)
	//{
	//	if (Input.DataType == "MidiStream")
	//		MidiOutputNames.Add(Name.ToString());
	//}
	return MidiOutputNames;
}