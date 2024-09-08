// Fill out your copyright notice in the Description page of Project Settings.

#include "SequenceDataFactory/BKMusicSequenceDataFactory.h"
#include "MidiDrivensSequenceEditor/MidiDrivenSequenceFactory.h"
#include "M2SoundEdGraphSchema.h"

UObject* UBKMusicSequenceDataFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	auto NewSequence = NewObject<UDAWSequencerData>(InParent, InClass, InName, Flags);
	NewSequence->M2SoundGraph = NewObject<UM2SoundGraph>(NewSequence, FName(), RF_Transactional);
	NewSequence->M2SoundGraph->Schema = UM2SoundEdGraphSchema::StaticClass();

	if (CallingContext == TEXT("ContentBrowserNewAsset"))
	{
		auto NewMidiFile = NewObject<UMidiFile>(NewSequence, FName(), RF_Transactional);

		NewMidiFile->BuildConductorTrack();
		NewMidiFile->GetSongMaps()->GetTempoMap().AddTempoInfoPoint(Harmonix::Midi::Constants::BPMToMidiTempo(75), 0);
		NewMidiFile->GetSongMaps()->GetBarMap().AddTimeSignatureAtBarIncludingCountIn(0, 4, 4, true);

		NewMidiFile->SortAllTracks();

		//NewSequence->HarmonixMidiFile = NewMidiFile;

		NewSequence->PopulateFromMidiFile(NewMidiFile);
	}

	UMidiDrivenSequenceFactory* SequenceFactory = NewObject<UMidiDrivenSequenceFactory>();

	//NewSequence->MidiDrivenLevelSequence = Cast<UMidiDrivenLevelSequence>(SequenceFactory->FactoryCreateNew(UMidiDrivenLevelSequence::StaticClass(), NewSequence, InName, Flags, Context, Warn));

	//NewSequence->M2SoundGraph->InitializeGraph();

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

			if (SequenceData->PlayState == TransportPlaying)
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