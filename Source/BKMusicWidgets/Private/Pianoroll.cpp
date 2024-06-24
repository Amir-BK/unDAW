// Fill out your copyright notice in the Description page of Project Settings.


#include "Pianoroll.h"

TSharedRef<SWidget> UPianoroll::RebuildWidget()
{
	UDAWSequencerData* SequencerData;

	if(SceneManager && SceneManager->GetDAWSequencerData())
	{
		SequencerData = SceneManager->GetDAWSequencerData();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SceneManager or SequencerData is null!"));
		SequencerData = nullptr;
	}
	
	SAssignNew(PianoRollGraph, SPianoRollGraph)
		.SessionData(SequencerData)
		.Clipping(EWidgetClipping::ClipToBounds);

	
	PianoRollGraph->bFollowCursor = false;
	PianoRollGraph->Init();

	if(SceneManager)
	{
		PianoRollGraph->SetCurrentTimestamp(SceneManager->CurrentTimestamp);
	}


	//test connection to on mouse button down lambda, just print something
	PianoRollGraph->OnMouseButtonDownDelegate.BindLambda([](const FGeometry& Geometry, const FPointerEvent& PointerEvent) { UE_LOG(LogTemp, Warning, TEXT("Mouse button down!")); return FReply::Handled(); });

	return PianoRollGraph.ToSharedRef();
}
