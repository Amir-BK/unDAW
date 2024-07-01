// Fill out your copyright notice in the Description page of Project Settings.


#include "Pianoroll.h"
#include "M2SoundGraphStatics.h"

TSharedRef<SWidget> UPianoroll::RebuildWidget()
{

	if(SceneManager && SceneManager->GetDAWSequencerData())
	{
		DawSequencerData = SceneManager->GetDAWSequencerData();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SceneManager or SequencerData is null, creating transient session, this should only happen with the UMG Designer"));
		//SequencerData = UM2SoundGraphStatics::CreateEmptySequencerData();
	}


	
	SAssignNew(PianoRollGraph, SPianoRollGraph)
		.SessionData(DawSequencerData)
		.Clipping(EWidgetClipping::ClipToBounds)
		.PianoTabWidth(PianoTabMargin)
		.CursorFollowAnchorPosition(DesiredCursorPositionOnScreen);

	
	PianoRollGraph->bFollowCursor = true;
	//PianoRollGraph->Init();

	if(SceneManager)
	{
		//PianoRollGraph->SetCurrentTimestamp(SceneManager->CurrentTimestamp);
	}


	//test connection to on mouse button down lambda, just print something
	PianoRollGraph->OnMouseButtonDownDelegate.BindLambda([](const FGeometry& Geometry, const FPointerEvent& PointerEvent) { UE_LOG(LogTemp, Warning, TEXT("Mouse button down!")); return FReply::Handled(); });

	return PianoRollGraph.ToSharedRef();
}

void UPianoroll::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	SequencerData = nullptr;
	PianoRollGraph.Reset();
}
