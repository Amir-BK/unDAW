// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EngineGlobals.h"
#include "TimeSyncedPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "TrackPlaybackAndDisplayOptions.h"
#include "UObject/Object.h"
#include "ISinglePropertyView.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "M2SoundGraphData.h"
#include "M2SoundEdGraphSchema.h"


// this are global settings affecting the entire editor
class SMidiEditorBaseSettingsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMidiEditorBaseSettingsWidget)
		{}
		SLATE_ARGUMENT(TSharedPtr<ITimeSyncedPanel>, parentMidiEditor)

	SLATE_END_ARGS()

	TSharedPtr<ITimeSyncedPanel> parentMidiEditor;

	void Construct(const FArguments& InArgs)
	{
		parentMidiEditor = InArgs._parentMidiEditor;

		ChildSlot[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(EVerticalAlignment::VAlign_Center)

				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Snapping Step Size")))
				]
		];
	}
};

/**
 * slate widget that allows controlling tracks on a pianoroll, individual track controls (such as visibility, color, Z-order, selection)
 */

class SMIDITrackControls : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMIDITrackControls)
		{}

		SLATE_ARGUMENT(int, slotInParentID)
		SLATE_ARGUMENT(FText, trackName)
		SLATE_ARGUMENT(FTrackDisplayOptions*, TrackData)
		SLATE_ARGUMENT(UM2SoundGraphBase*, ConnectedGraph)
		SLATE_ARGUMENT(UDAWSequencerData*, SequencerData)
		//	SLATE_EVENT(FOnFusionPatchChanged, OnFusionPatchChanged)

	SLATE_END_ARGS()

	int slotInParentID;
	FText trackName;

	TSharedPtr<FString> CurrentItem;
	TArray<TSharedPtr<FString>> optionsArray;
	FTrackDisplayOptions* TrackData;
	//FOnFusionPatchChanged OnFusionPatchChanged;
	UM2SoundGraphBase* ConnectedGraph;
	UDAWSequencerData* SequencerData;

	void toggleTrackVisibility(ECheckBoxState newState)
	{
		//parentMidiEditor->ToggleTrackVisibility(slotInParentID, newState == ECheckBoxState::Checked ? true : false);
	}

	void SetColor(FLinearColor newColor)
	{
		//parentMidiEditor->GetTracksDisplayOptions(slotInParentID).trackColor = newColor;
		TrackData->TrackColor = newColor;
		if (ConnectedGraph) ConnectedGraph->NotifyGraphChanged();
	}

	void SelectionCancel(FLinearColor newColor)
	{
		TrackData->TrackColor = newColor;
		if (ConnectedGraph) ConnectedGraph->NotifyGraphChanged();
	}

	FReply SelectThisTrack()
	{
		//parentMidiEditor->SelectTrack(slotInParentID);
		if (SequencerData->SelectedTrackIndex != slotInParentID)
		{
			SequencerData->SelectedTrackIndex = slotInParentID;
		}
		else
		{
			SequencerData->SelectedTrackIndex = -1;
		}
		return FReply::Handled();
	}

	FReply TrackOpenColorPicker(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			FColorPickerArgs PickerArgs;
			PickerArgs.bOnlyRefreshOnMouseUp = false;
			PickerArgs.ParentWidget = AsShared();
			PickerArgs.bUseAlpha = false;
			PickerArgs.bOnlyRefreshOnOk = true;
			PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
			PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SMIDITrackControls::SetColor);

			PickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &SMIDITrackControls::SelectionCancel);
			PickerArgs.InitialColor = TrackData->TrackColor;
			OpenColorPicker(PickerArgs);
		}

		return FReply::Handled();
	}

	TSharedRef<SWidget> MakeWidgetForOption(TSharedPtr<FString> InOption)
	{
		return SNew(STextBlock).Text(FText::FromString(*InOption));
	}


	FText GetCurrentItemLabel() const
	{
		if (CurrentItem.IsValid()) return FText::FromString(*CurrentItem);
		return FText::FromString(TEXT("No Fusion Patch Selected"));
	}

	void Construct(const FArguments& InArgs)
	{
		//parentMidiEditor = InArgs._parentMidiEditor;
		slotInParentID = InArgs._slotInParentID;
		TrackData = InArgs._TrackData;
		FString FusionPatchName = TEXT("DEPRECATED");
		CurrentItem = MakeShareable(new FString(FusionPatchName));
		//OnFusionPatchChanged = InArgs._OnFusionPatchChanged;
		ConnectedGraph = InArgs._ConnectedGraph;
		SequencerData = InArgs._SequencerData;


		ChildSlot
			[
				SNew(SBorder)
					//.Padding(2.0f)
					[
						SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()

							.VAlign(EVerticalAlignment::VAlign_Center)
							[

								SNew(STextBlock)
									.Text(FText::Format(INVTEXT("{0}"), InArgs._slotInParentID))

							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									[
										SNew(SEditableTextBox)
											.Text(FText::FromString(TrackData->TrackName))
									]

									+SVerticalBox::Slot()
									[
										SNew(SHorizontalBox)
											+ SHorizontalBox::Slot()
											[
												SNew(SCheckBox)
													.IsChecked(ECheckBoxState::Checked)
													.OnCheckStateChanged_Raw(this, &SMIDITrackControls::toggleTrackVisibility)
											]
											+ SHorizontalBox::Slot()
											[
												SNew(SButton)
													.Text(FText::FromString(TEXT("Select")))
													.OnClicked(this, &SMIDITrackControls::SelectThisTrack)

											]
											+ SHorizontalBox::Slot()
											[
												SNew(SColorBlock)
													.Color(TAttribute<FLinearColor>::Create(TAttribute<FLinearColor>::FGetter::CreateLambda([&]() {
													return TrackData->TrackColor; //parentMidiEditor->GetTracksDisplayOptions(slotInParentID).trackColor;
														})))
	
													.OnMouseButtonDown(this, &SMIDITrackControls::TrackOpenColorPicker)
											]

									]
							]
					]

			];
	};
};