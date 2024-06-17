// Fill out your copyright notice in the Description page of Project Settings.


#include "UnDAWPreviewHelperSubsystem.h"
#include "M2SoundGraphRenderer.h"


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

void UUnDAWPreviewHelperSubsystem::MapMetasoundPatchPinValues(UMetaSoundPatch* InPatch)
{
	//UE_LOG(unDAWVertexLogs, Verbose, TEXT("SaveDefaultsToVertexCache %s"), *Patch->GetName())
		//if we have a valid vertex cache, save the defaults to it


	//UUNDAWSettings::Get()->Cache.Add(Patch->GetFName(), FCachedVertexPinInfo());
	//UUNDAWSettings::Get()->SaveConfig();

	auto& Document = InPatch->GetDocumentChecked();
    /*
	TArray<UMetasoundEditorGraphMemberDefaultFloat*> FloatLiterals;
	InPatch->GetGraph()->GetNodesOfClass<UMetasoundEditorGraphMemberDefaultFloat>(FloatLiterals);

    for(auto& FloatLiteral : FloatLiterals)
	{
		//print range!!!
        auto RangeMin = FloatLiteral->Range.GetMin();
        auto RangeMax = FloatLiteral->Range.GetMax();

        UE_LOG(LogTemp, Verbose, TEXT("FloatLiteral %s %f %f"), *FloatLiteral->GetFName().ToString(), RangeMin, RangeMax);
	}
    */

}


