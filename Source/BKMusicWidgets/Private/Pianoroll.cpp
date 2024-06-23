// Fill out your copyright notice in the Description page of Project Settings.


#include "Pianoroll.h"

TSharedRef<SWidget> UPianoroll::RebuildWidget()
{
	SAssignNew(PianoRollGraph, SPianoRollGraph)
		.SessionData(SceneManager->GetDAWSequencerData())
		.Clipping(EWidgetClipping::ClipToBounds)
		.gridColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("8A8A8A00"))))
		.accidentalGridColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("00000082"))))
		.cNoteColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FF33E220"))));

	//not CLEVER!!!! Should not be done like this, the scene manager needs to update its own timestamp for all potential listeners!
	SceneManager->CurrentTimestamp = SceneManager->CurrentTimestamp.CreateLambda([this]() { return SceneManager->GetDAWSequencerData() ? SceneManager->GetDAWSequencerData()->CurrentTimestampData : FMusicTimestamp(); });
	
	PianoRollGraph->bFollowCursor = false;
	PianoRollGraph->Init();
	PianoRollGraph->OnSeekEvent.BindUObject(SceneManager->GetDAWSequencerData(), &UDAWSequencerData::SendSeekCommand);
	PianoRollGraph->SetCurrentTimestamp(SceneManager->CurrentTimestamp);

	return PianoRollGraph.ToSharedRef();
}
