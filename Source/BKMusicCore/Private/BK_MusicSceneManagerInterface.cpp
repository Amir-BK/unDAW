// Fill out your copyright notice in the Description page of Project Settings.


#include "BK_MusicSceneManagerInterface.h"


// Add default functionality here for any IBK_MusicSceneManagerInterface functions that are not pure virtual.

BKMUSICCORE_API DEFINE_LOG_CATEGORY(BKMusicInterfaceLogs);

void IBK_MusicSceneManagerInterface::SendTransportCommand(EBKTransportCommands InCommand)
{
	//TODODOTOD	TODTO
	
	UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("%s (%s): ReceivedCommand: %s, "), *this->_getUObject()->GetFullName(), this->_getUObject()->GetWorld() ? *this->_getUObject()->GetWorld()->GetName() : TEXT("No world"), *UEnum::GetValueAsString(InCommand))

	switch (InCommand)
		{
		
		case Init:
			// create builder

			

			InitializeAudioBlock();
			break;
		case Play:
			
			UE_LOG(LogTemp, Log, TEXT("Received Play Command, Current Playback State %s"), *UEnum::GetValueAsString(GetCurrentPlaybackState()))
			switch (GetCurrentPlaybackState())
			{
			case ReadyToPlay:
				if (GetAudioComponent())
				{
					GetAudioComponent()->Play();
					GetAudioComponent()->SetTriggerParameter(FName("unDAW.Transport.Play"));
					SetPlaybackState(Playing);
				}
				break;


			default:

				// This is hack - must figure this out
				if (GetAudioComponent())
				{
					GetAudioComponent()->Play();
					GetAudioComponent()->SetTriggerParameter(FName("unDAW.Transport.Play"));
					SetPlaybackState(Playing);
				}
				break;


			}
			//UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("%s: Audio Component: %s, "), *this->_getUObject()->GetName(), *GetAudioComponent()->GetName())
			//GetAudioComponent()->Activate();

		
			//GetAudioComponent()->SetTriggerParameter(FName("Play"));
			//GetAudioComponent()->SetTriggerParameter(FName("UE.Source.OnPlay"));
			//GetAudioComponent()->SetTriggerParameter(FName("Prepare"));
			//UE.Source.OnPlay
			
			//UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("Received Play"))
			
				break;
		case Pause:
			SetPlaybackState(EBKPlayState::Paused);
			GetAudioComponent()->SetTriggerParameter(FName("unDAW.Transport.Pause"));
			SetPlaybackState(Paused);
			break;
		case Stop:
			SetPlaybackState(EBKPlayState::ReadyToPlay);
			GetAudioComponent()->SetTriggerParameter(FName("unDAW.Transport.Stop"));
			break;
		case Kill:
			GetAudioComponent()->SetTriggerParameter(FName("unDAW.Transport.Kill"));
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

void IBK_MusicSceneManagerInterface::SetPlayrate(float newPlayrate)
{
	GetAudioComponent()->SetFloatParameter(FName("unDAW.Midi.Speed"), newPlayrate);
	Playrate = newPlayrate;
}

UMetasoundBuilderHelperBase* IBK_MusicSceneManagerInterface::InitializeAudioBlock()
{
	UMetasoundBuilderHelperBase* BuilderHelperInstance = NewObject<UMetasoundBuilderHelperBase>(this->_getUObject(), GetBuilderBPClass());
	BuilderHelperInstance->SessionData = GetActiveSessionData();
	BuilderHelperInstance->InitBuilderHelper(TEXT("unDAW_Session_Builder"));

	SetBuilderHelper(BuilderHelperInstance);


	
	//UAudioComponent* component = GetAudioComponent();
	//if (GetAudioComponent()) GetAudioComponent()->Stop();
	
	GeneratorCreated.BindDynamic(this, &IBK_MusicSceneManagerInterface::OnMetasoundHandleGenerated);

	FMetaSoundBuilderOptions options = FMetaSoundBuilderOptions();
	options.bForceUniqueClassName = true;


	//BuilderHelperInstance->GeneratedMetaSound = BuilderHelperInstance->CurrentBuilder->Build(this->_getUObject(), options);
	// 
	//hopefully this will get us the opportunity to create an audio component in BP.
	Entry_Initializations();
	SetPlaybackState(EBKPlayState::ReadyToPlay);

	//BuilderHelperInstance->CurrentBuilder->Audition(this->_getUObject(), GetAudioComponent(), GeneratorCreated, true);


	return BuilderHelperInstance;
}

void IBK_MusicSceneManagerInterface::OnMetasoundHandleGenerated(UMetasoundGeneratorHandle* GeneratorHandle)
{
	GetAudioComponent()->SetObjectParameter(FName("MidiFile"), GetActiveSessionData()->TimeStampedMidis[0].MidiFile);
	GetAudioComponent()->SetTriggerParameter(FName("Prepare"));
	
	//we call this overriden function to let us 
	
	SetGeneratorHandle(GeneratorHandle);

	UE_LOG(BKMusicInterfaceLogs, Verbose, TEXT("%s : Generated Handle, Generator Handle name %s"), *this->_getUObject()->GetName(), *GeneratorHandle->GetName())
	//return inFromDelegate;
}
