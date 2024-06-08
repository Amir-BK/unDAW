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
#include "SequencerData.h"
#include "M2SoundEdGraphSchema.h"
#include "UnDAWSFZAsset.h"


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
		SLATE_ARGUMENT(UM2SoundGraphBase*, ConnectedGraph )
		SLATE_ARGUMENT(UDAWSequencerData*, SequencerData )
		SLATE_EVENT(FOnFusionPatchChanged, OnFusionPatchChanged)

	SLATE_END_ARGS()

	int slotInParentID;
	FText trackName;

	TSharedPtr<FString> CurrentItem;
	TArray<TSharedPtr<FString>> optionsArray;
	FTrackDisplayOptions* TrackData;
	FOnFusionPatchChanged OnFusionPatchChanged;
	UM2SoundGraphBase* ConnectedGraph;
	UDAWSequencerData* SequencerData;

	void toggleTrackVisibility(ECheckBoxState newState)
	{
		//parentMidiEditor->ToggleTrackVisibility(slotInParentID, newState == ECheckBoxState::Checked ? true : false);
	}

	void SetColor(FLinearColor newColor)
	{
		//parentMidiEditor->GetTracksDisplayOptions(slotInParentID).trackColor = newColor;
		TrackData->trackColor = newColor;
		if(ConnectedGraph) ConnectedGraph->NotifyGraphChanged();
	}

	void SelectionCancel(FLinearColor newColor)
	{
		TrackData->trackColor = newColor;
		if(ConnectedGraph) ConnectedGraph->NotifyGraphChanged();
	}

	FReply SelectThisTrack()
	{
		//parentMidiEditor->SelectTrack(slotInParentID);

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
			PickerArgs.InitialColor = TrackData->trackColor;
			OpenColorPicker(PickerArgs);

		}

		return FReply::Handled();
	}

	TSharedRef<SWidget> MakeWidgetForOption(TSharedPtr<FString> InOption)
	{
		return SNew(STextBlock).Text(FText::FromString(*InOption));
	}


	void OnSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type)
	{
		if(CurrentItem == NewValue) return;
		
		CurrentItem = NewValue;

		if (CurrentItem.IsValid())
		{
			for (auto& patch : UFKSFZAsset::GetAllFusionPatchAssets())
			{
				if (patch->GetName().Equals(*CurrentItem, ESearchCase::IgnoreCase)) 
				{
						//TrackData->fusionPatch = TObjectPtr<UFusionPatch>(patch);
						OnFusionPatchChanged.ExecuteIfBound(slotInParentID, patch);
						//parentMidiEditor->GetTracksDisplayOptions(slotInParentID).SampleAvailabilityMap.Empty();
						for (int i = 0; i < 127; i++)
						{
							for (auto& keyzone : patch->GetKeyzones())
							{
								if (i >= keyzone.MinNote && i <= keyzone.MaxNote)
								{
									//parentMidiEditor->GetTracksDisplayOptions(slotInParentID).SampleAvailabilityMap.Add(TTuple<int, bool>(i, true));
									break;
								}
							}
						}
						//parentMidiEditor->currentTimelineCursorPosition
						break;
				}
			}

		}
	
		
	}

	FText GetCurrentItemLabel() const
	{
		if(CurrentItem.IsValid()) return FText::FromString(*CurrentItem);
		return FText::FromString(TEXT("No Fusion Patch Selected"));
	}
	


	void Construct(const FArguments& InArgs) 
	{
		//parentMidiEditor = InArgs._parentMidiEditor;
		slotInParentID = InArgs._slotInParentID;
		TrackData = InArgs._TrackData;
		CurrentItem = MakeShareable(new FString(TrackData->fusionPatch->GetName()));
		OnFusionPatchChanged = InArgs._OnFusionPatchChanged;
		ConnectedGraph = InArgs._ConnectedGraph;
		SequencerData	= InArgs._SequencerData;

		//if (parentMidiEditor->GetTracksDisplayOptions(slotInParentID).fusionPatch != nullptr)
		//{
		//	CurrentItem = MakeShareable(new FString(parentMidiEditor->GetTracksDisplayOptions(slotInParentID).fusionPatch->GetName()));
		//}
		//else {
		//	CurrentItem = MakeShareable(new FString(TEXT("Please select fusion patch")));
		//}
		

		for (auto& patch : UFKSFZAsset::GetAllFusionPatchAssets())
		{
			if (TrackData->fusionPatch == patch) {
				CurrentItem = MakeShareable(new FString(patch->GetName()));
				optionsArray.Add(CurrentItem);
			}
			else {
			if(IsValid(patch)) optionsArray.Add(MakeShareable(new FString(patch->GetName())));
			}
		}

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
								.Text(FText::FromString(TrackData->trackName))
								//.Text(FText::FromString(FString::Printf(TEXT("%s %d ch: %d"), *parentMidiEditor->GetTracksDisplayOptions(slotInParentID).trackName, parentMidiEditor->GetTracksDisplayOptions(slotInParentID).TrackIndexInParentMidi, parentMidiEditor->GetTracksDisplayOptions(slotInParentID).ChannelIndexInParentMidi)))
							//	.Text(FText::FromString(parentMidiEditor->GetTracksDisplayOptions(slotInParentID).trackName))
								//.OnTextCommitted_Lambda([this](const FText& newText, ETextCommit::Type commitType) {parentMidiEditor->GetTracksDisplayOptions(slotInParentID).trackName = newText.ToString(); })
						]

						+ SVerticalBox::Slot()
						[
							SNew(SComboBox<TSharedPtr<FString>>)
								.OptionsSource(&optionsArray)
								.OnGenerateWidget(this, &SMIDITrackControls::MakeWidgetForOption)
								.OnSelectionChanged(this, &SMIDITrackControls::OnSelectionChanged)
	
								.InitiallySelectedItem(CurrentItem)
								[

									SNew(STextBlock)
										.Text_Lambda([this]() {return GetCurrentItemLabel(); })

								]
						]


						+ SVerticalBox::Slot()
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

										return TrackData->trackColor; //parentMidiEditor->GetTracksDisplayOptions(slotInParentID).trackColor;


											})))
										//.Size(FVector2D(350.0f, 20.0f))
										.OnMouseButtonDown(this, &SMIDITrackControls::TrackOpenColorPicker)
								]

						]
				]
				]
			
			];
	};

};