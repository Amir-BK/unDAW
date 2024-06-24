// Fill out your copyright notice in the Description page of Project Settings.


#include "SequenceDataFactory/BKMusicSequenceDataFactory.h"
#include "M2SoundEdGraphSchema.h"

UObject* UBKMusicSequenceDataFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	auto NewSequence =	NewObject<UDAWSequencerData>(InParent, InClass, InName, Flags);

	NewSequence->M2SoundGraph = NewObject<UM2SoundGraph>(NewSequence, FName(), RF_Transactional);
	NewSequence->M2SoundGraph->Schema = UM2SoundEdGraphSchema::StaticClass();

	auto NewMidiFile = NewObject<UMidiFile>(NewSequence, FName(), RF_Transactional);

	//init file, might be a bit wasteful to do it here but it's fine for now
	//NewMidiFile->GetSongMaps()->GetBarMap().SetTicksPerQuarterNote(480);


	//add a note on and off events to track 0, pitch 60, velocity 100, at time 0
	auto NoteOnMidiMessage = FMidiMsg::CreateNoteOn(1, 60, 100);
	auto NoteOffMidiMessage = FMidiMsg::CreateNoteOff(1, 60);

	auto NoteOnEvent = FMidiEvent(0, NoteOnMidiMessage);
	auto NoteOffEvent = FMidiEvent(480, NoteOffMidiMessage);

	//another note on and off event for testing
	auto NoteOnMidiMessage2 = FMidiMsg::CreateNoteOn(0, 62, 100);
	auto NoteOffMidiMessage2 = FMidiMsg::CreateNoteOff(0, 62);

	auto NoteOnEvent2 = FMidiEvent(0, NoteOnMidiMessage2);
	auto NoteOffEvent2 = FMidiEvent(480, NoteOffMidiMessage2);



	NewMidiFile->BuildConductorTrack();
	NewMidiFile->GetSongMaps()->GetTempoMap().AddTempoInfoPoint(Harmonix::Midi::Constants::BPMToMidiTempo(140), 0);
	NewMidiFile->GetSongMaps()->GetBarMap().AddTimeSignatureAtBarIncludingCountIn(0, 4, 4, true);


	auto NewTrack = NewMidiFile->AddTrack(FString::Printf(TEXT("New Track %d"), 0));
	//NewTrack->AddEvent(NoteOnEvent);
	//NewTrack->AddEvent(NoteOffEvent);

	//NewTrack->AddEvent(NoteOnEvent2);
	//NewTrack->AddEvent(NoteOffEvent2);

	



	//ConductorTrack->AddEvent(DefaultTempoEvent);
	//ConductorTrack->AddEvent(DefaultTimeSigEvent);

	NewMidiFile->SortAllTracks();

	NewSequence->HarmonixMidiFile = NewMidiFile;

	return NewSequence;
}

bool UBKMusicSequenceDataFactory::ShouldShowInNewMenu() const
{
	return true;
}

UBKMusicSequenceDataFactory::UBKMusicSequenceDataFactory()
{
	SupportedClass = UDAWSequencerData::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}


// Next time I come edit this shit I gotta remember it gets overriden... 

TSharedPtr<SWidget> FDAWSequenceAssetActions::GetThumbnailOverlay(const FAssetData& InAssetData) const
{
	auto OnClickedLambda = [InAssetData]() -> FReply
		{
			auto SequenceData = Cast<UDAWSequencerData>(InAssetData.GetAsset());
			
			if (SequenceData->PlayState == TransportPlaying )
			{
				//unDAW::PreviewPlayback::StopSound();
				SequenceData->SendTransportCommand(EBKTransportCommands::Stop);
			}
			else
			{
				// Load and play sound
				auto PreviewHelper = GEditor->GetEditorSubsystem<UUnDAWPreviewHelperSubsystem>();
				//PreviewHelper->OnDAWPerformerReady
				
				if (SequenceData->PlayState == ReadyToPlay)
				{
					SequenceData->SendTransportCommand(EBKTransportCommands::Play);
					//SequenceData->MetasoundBuilderHelper->AuditionComponentRef->SetTriggerParameter(FName("unDAW.Transport.Play"));
					return FReply::Handled();
				}
				PreviewHelper->CreateAndPrimePreviewBuilderForDawSequence(SequenceData);
				SequenceData->SendTransportCommand(EBKTransportCommands::Play);

				//SequenceData->MetasoundBuilderHelper->OnDAWPerformerReady.AddLambda([](UDAWSequencerData* Data)
				//{
				//		FDAWSequenceAssetActions::TryTriggerAudioPlay(Data);
				//});
				//unDAW::PreviewPlayback::PlaySound(Cast<USoundBase>(InAssetData.GetAsset()));
			}
			return FReply::Handled();
		};
	return GetDawSequenceThumbnailOverlay(InAssetData, MoveTemp(OnClickedLambda));
}
