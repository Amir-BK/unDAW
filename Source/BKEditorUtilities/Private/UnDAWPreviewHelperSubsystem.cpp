// Fill out your copyright notice in the Description page of Project Settings.

#include "UnDAWPreviewHelperSubsystem.h"
#include "Editor.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"

void UUnDAWPreviewHelperSubsystem::CreateAndPrimePreviewBuilderForDawSequence(UDAWSequencerData* InSessionToPreview, bool bForceReinit)
{

	if (InSessionToPreview->AuditionComponent && InSessionToPreview->AuditionComponent->IsPlaying())
	{
		//ugh it was already setup? 
		if (InSessionToPreview->AuditionComponent->Sound.Get() == Cast<USoundBase>(InSessionToPreview->SavedMetaSound.Get()))
		{
			//we are already playing the correct sound
			if (bForceReinit) InSessionToPreview->AuditionComponent->Stop();
			//else
		}
		else
		{
			//we are playing the wrong sound

			//try to delete, kill with fire the transient metasound
			delete InSessionToPreview->AuditionComponent->Sound.Get();

			InSessionToPreview->AuditionComponent->Stop();
			InSessionToPreview->AuditionBuilder(InSessionToPreview->AuditionComponent, true);
			InSessionToPreview->AuditionComponent->SetSound(Cast<USoundBase>(InSessionToPreview->SavedMetaSound.Get()));
			InSessionToPreview->AuditionComponent->Play();
			return;
		}
	}
		
	

	if (!hasAlreadyPrimed)
	{
		auto PrimingSound = FSoftObjectPath(TEXT("/unDAW/BKSystems/Core/PrimingAudioDontMove/1kSineTonePing.1kSineTonePing")).TryLoad();
		GEditor->PlayPreviewSound(Cast<USoundWave>(PrimingSound));
		//hasAlreadyPrimed = true;
	}

	auto AudioComponent = GEditor->ResetPreviewAudioComponent();

	//if (!AudioComponent) return;

	AudioComponent->bAutoDestroy = false;
	AudioComponent->bIsUISound = true;
	AudioComponent->bAllowSpatialization = false;
	AudioComponent->bReverb = false;
	AudioComponent->bCenterChannelOnly = false;
	AudioComponent->bIsPreviewSound = true;
	//AudioComponent->SetIsVirtualized(true);
	//AudioComponent->Set

	;
	//InSessionToPreview->EditorPreviewPerformer = InSessionToPreview->CreatePerformer(AudioComponent);
	//InSessionToPreview->EditorPreviewPerformer->AuditionComponentRef = AudioComponent;
	//InSessionToPreview->EditorPreviewPerformer->CreateAuditionableMetasound(AudioComponent, true);
	//InSessionToPreview
	InSessionToPreview->AuditionBuilder(AudioComponent, true);

	ActivePreviewPerformer.ActiveSession = InSessionToPreview;
	//ActivePreviewPerformer.PreviewPerformer = InSessionToPreview->EditorPreviewPerformer;
}