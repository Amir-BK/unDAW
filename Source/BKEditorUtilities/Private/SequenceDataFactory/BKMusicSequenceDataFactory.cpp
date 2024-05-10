// Fill out your copyright notice in the Description page of Project Settings.


#include "SequenceDataFactory/BKMusicSequenceDataFactory.h"

UObject* UBKMusicSequenceDataFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UDAWSequencerData>(InParent, InClass, InName, Flags);
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

inline TSharedPtr<SWidget> FDAWSequenceAssetActions::GetThumbnailOverlay(const FAssetData& InAssetData) const
{
	auto OnClickedLambda = [InAssetData]() -> FReply
		{
			auto SequenceData = Cast<UDAWSequencerData>(InAssetData.GetAsset());
			
			if (SequenceData->EditorPreviewPerformer && SequenceData->EditorPreviewPerformer->PlayState == Playing )
			{
				//UE::AudioEditor::StopSound();
				SequenceData->EditorPreviewPerformer->SendTransportCommand(EBKTransportCommands::Stop);
			}
			else
			{
				// Load and play sound
				auto PreviewHelper = GEditor->GetEditorSubsystem<UUnDAWPreviewHelperSubsystem>();
				//PreviewHelper->OnDAWPerformerReady
				
				if (SequenceData->EditorPreviewPerformer && SequenceData->EditorPreviewPerformer->PlayState == ReadyToPlay)
				{
					SequenceData->EditorPreviewPerformer->SendTransportCommand(EBKTransportCommands::Play);
					//SequenceData->MetasoundBuilderHelper->AuditionComponentRef->SetTriggerParameter(FName("unDAW.Transport.Play"));
					return FReply::Handled();
				}
				PreviewHelper->CreateAndPrimePreviewBuilderForDawSequence(SequenceData);


				//SequenceData->MetasoundBuilderHelper->OnDAWPerformerReady.AddLambda([](UDAWSequencerData* Data)
				//{
				//		FDAWSequenceAssetActions::TryTriggerAudioPlay(Data);
				//});
				//UE::AudioEditor::PlaySound(Cast<USoundBase>(InAssetData.GetAsset()));
			}
			return FReply::Handled();
		};
	return GetDawSequenceThumbnailOverlay(InAssetData, MoveTemp(OnClickedLambda));
}
