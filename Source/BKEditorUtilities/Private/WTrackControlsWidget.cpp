// Fill out your copyright notice in the Description page of Project Settings.


#include "WTrackControlsWidget.h"
#include "SMidiTrackControlsWidget.h"



void UWTrackControlsWidget::InitFromData()
{
    tracksVerticalBox->ClearChildren();


    //for (const auto& [index, track] : MidiEditorSharedPtr->GetTrackDisplayOptions())
    //{
    //    tracksVerticalBox->AddSlot()
    //        .AutoHeight()
    //        [
    //            SNew(SMIDITrackControls)
    //                .trackName(FText::FromString(*track.trackName))
    //                .parentMidiEditor(MidiEditorSharedPtr)
    //                .slotInParentID(track.ChannelIndexInParentMidi)
    //        ];

    //}
}

TSharedRef<SWidget> UWTrackControlsWidget::RebuildWidget()
{
	tracksVerticalBox = SNew(SVerticalBox);

	return tracksVerticalBox.ToSharedRef();
}

void UWTrackControlsWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	//tracksVerticalBox->ClearChildren();
	tracksVerticalBox.Reset();
	//if (MidiEditorSharedPtr.IsValid()) MidiEditorSharedPtr.Reset();
}
