// Fill out your copyright notice in the Description page of Project Settings.


#include "BK_MusicSceneManagerInterface.h"


// Add default functionality here for any IBK_MusicSceneManagerInterface functions that are not pure virtual.

BKMUSICCORE_API DEFINE_LOG_CATEGORY(BKMusicInterfaceLogs);

void IBK_MusicSceneManagerInterface::CreatePerformer(UAudioComponent* InAudioComponent)
{

}

//void IBK_MusicSceneManagerInterface::SendTransportCommand(EBKTransportCommands InCommand)
//{
//	//TODODOTOD	TODTO
//	
//	UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("%s (%s): ReceivedCommand: %s, "), *this->_getUObject()->GetFullName(), this->_getUObject()->GetWorld() ? *this->_getUObject()->GetWorld()->GetName() : TEXT("No world"), *UEnum::GetValueAsString(InCommand))
//	UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("Received transport Command, Current Playback State %s"), *UEnum::GetValueAsString(GetCurrentPlaybackState()))
//	
//}

void IBK_MusicSceneManagerInterface::SetPlayrate(float newPlayrate)
{
	//GetAudioComponent()->SetFloatParameter(FName("unDAW.Midi.Speed"), newPlayrate);
	Playrate = newPlayrate;
}

void IBK_MusicSceneManagerInterface::SendSeekCommand(float InSeek)
{
	//switch (GetCurrentPlaybackState())
	//{
	//case Playing:
	//	if (GetAudioComponent())
	//	{
	//		//SetPlaybackState(EBKPlayState::ReadyToPlay);
	//		GetAudioComponent()->SetFloatParameter(FName("unDAW.Transport.SeekTarget"), InSeek * 1000.f);
	//		GetAudioComponent()->SetTriggerParameter(FName("unDAW.Transport.Seek"));
	//		//SetPlaybackState(Re);
	//	}
	//	break;


	//default:

	//	UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("Some other state"))
	//		break;

	//}

	UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("Received Seek Command"))
}




