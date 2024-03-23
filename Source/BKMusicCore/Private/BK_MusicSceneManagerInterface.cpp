// Fill out your copyright notice in the Description page of Project Settings.


#include "BK_MusicSceneManagerInterface.h"


// Add default functionality here for any IBK_MusicSceneManagerInterface functions that are not pure virtual.

void UDAWSequencerData::CalculateSequenceDuration()
{
	if (!TimeStampedMidis.IsEmpty())
	{
		SequenceDuration = TimeStampedMidis[0].MidiFile->GetSongMaps()->GetSongLengthMs();
	}
}

void IBK_MusicSceneManagerInterface::SendTransportCommand(EBKTransportCommands InCommand)
{
	//TODODOTOD	TODTO
	UE_LOG(LogTemp, Log, TEXT("But do we enter here?"))

		switch (InCommand)
		{
		case Init:
			// create builder


			Entry_Initializations();
			UE_LOG(LogTemp, Log, TEXT("Received Init"))
				break;
		case Play:

			UE_LOG(LogTemp, Log, TEXT("Received Play"))
				break;
		case Pause:
			break;
		case Stop:
			break;
		case Kill:
			break;
		case TransportBackward:
			break;
		case TransportForward:
			break;
		case NextMarker:
			break;
		case PrevMarker:
			break;
		default:
			break;
		}
}
