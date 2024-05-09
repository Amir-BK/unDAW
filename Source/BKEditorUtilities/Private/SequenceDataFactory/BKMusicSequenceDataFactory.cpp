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

inline TSharedPtr<SWidget> FDAWSequenceAssetActions::GetThumbnailOverlay(const FAssetData& InAssetData) const
{
	auto OnClickedLambda = [InAssetData]() -> FReply
		{
			if (UE::AudioEditor::IsSoundPlaying(InAssetData))
			{
				UE::AudioEditor::StopSound();
			}
			else
			{
				// Load and play sound
				auto PreviewHelper = GEditor->GetEditorSubsystem<UUnDAWPreviewHelperSubsystem>();
				//PreviewHelper->OnDAWPerformerReady
				auto SequenceData = Cast<UDAWSequencerData>(InAssetData.GetAsset());
				if (SequenceData->MetasoundBuilderHelper && SequenceData->MetasoundBuilderHelper->AuditionComponentRef)
				{
					SequenceData->MetasoundBuilderHelper->AuditionComponentRef->SetTriggerParameter(FName("unDAW.Transport.Play"));
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
