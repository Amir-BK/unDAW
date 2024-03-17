// Fill out your copyright notice in the Description page of Project Settings.


#include "WTrackControlsWidget.h"
#include "SMidiTrackControlsWidget.h"

void UWTrackControlsWidget::SetMidiEditorParentWidget(UMIDIEditorBase* inEditor)
{
	MidiEditorSharedPtr = inEditor;
	ParentMidiEditor = inEditor;
}

void UWTrackControlsWidget::InitFromData()
{
	tracksVerticalBox->ClearChildren();
	int TrackIndex = 0;
	for (auto& track : MidiEditorSharedPtr->GetTrackDisplayOptions())
	{
		tracksVerticalBox->AddSlot()
			.AutoHeight()
			[
				SNew(SMIDITrackControls)
					.trackName(FText::FromString(*track.trackName)) //midiTrack->TrackName))
					.parentMidiEditor(MidiEditorSharedPtr)
					.slotInParentID(TrackIndex)
			];

		TrackIndex++;
	}


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
