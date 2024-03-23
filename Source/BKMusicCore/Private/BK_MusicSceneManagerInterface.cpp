// Fill out your copyright notice in the Description page of Project Settings.


#include "BK_MusicSceneManagerInterface.h"


// Add default functionality here for any IBK_MusicSceneManagerInterface functions that are not pure virtual.



void IBK_MusicSceneManagerInterface::SendTransportCommand(EBKTransportCommands InCommand)
{
	//TODODOTOD	TODTO
		switch (InCommand)
		{
		case Init:
			// create builder


			Entry_Initializations();
			UE_LOG(LogTemp, Log, TEXT("Received Init"))

				InitializeAudioBlock();
				break;
		case Play:
			if (!GetAudioComponent()->IsPlaying()) GetAudioComponent()->Play();

			GetAudioComponent()->SetTriggerParameter(FName("Play"));
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

UMetasoundBuilderHelperBase* IBK_MusicSceneManagerInterface::InitializeAudioBlock()
{
	UMetasoundBuilderHelperBase* BuilderHelperInstance = NewObject<UMetasoundBuilderHelperBase>(this->_getUObject(), GetBuilderBPClass());
	BuilderHelperInstance->SessionData = GetActiveSessionData();
	BuilderHelperInstance->InitBuilderHelper(TEXT("unDAW_Session_Builder"));

	FOnCreateAuditionGeneratorHandleDelegate GeneratorCreated;
	//UAudioComponent* component = GetAudioComponent();
	GetAudioComponent()->Stop();
	BuilderHelperInstance->CurrentBuilder->Audition(this->_getUObject(), GetAudioComponent(), GeneratorCreated);
	GetAudioComponent()->SetObjectParameter(FName("MidiFile"), GetActiveSessionData()->TimeStampedMidis[0].MidiFile);
	GetAudioComponent()->SetTriggerParameter(FName("Prepare"));

	return BuilderHelperInstance;
}
