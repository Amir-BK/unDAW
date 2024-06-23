// Fill out your copyright notice in the Description page of Project Settings.


#include "UnDAWPreviewHelperSubsystem.h"



#include "MetasoundEditorGraphInputNode.h"



void UUnDAWPreviewHelperSubsystem::CreateAndPrimePreviewBuilderForDawSequence(UDAWSequencerData* InSessionToPreview)
{
	
   // if (ActivePreviewPerformer.ActiveSession && InSessionToPreview != ActivePreviewPerformer.ActiveSession)
    //{

			//ActivePreviewPerformer.PreviewPerformer->SendTransportCommand(EBKTransportCommands::Stop);
           // ActivePreviewPerformer.ActiveSession->EditorPreviewPerformer = nullptr;
           // ActivePreviewPerformer.PreviewPerformer->RemoveFromParent();
           // ActivePreviewPerformer.PreviewPerformer->ConditionalBeginDestroy();
	//}
    
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




