// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vertexes/M2VariMixerVertex.h"
#include "M2SoundGraphData.h"
#include "SAudioRadialSlider.h"
#include "SAudioSlider.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SCheckBox.h"

class BKMUSICWIDGETS_API SMixerChannelWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMixerChannelWidget)
		{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UM2VariMixerVertex* InMixerVertex, uint8 InChannelIndex);

	void Construct(const FArguments& InArgs, UM2AudioTrackPin* InPin);

	UM2VariMixerVertex* MixerVertex;
	UM2AudioTrackPin* Pin;
	uint8 ChannelIndex;

	TSharedPtr<SAudioRadialSlider> RadialSlider;
	TSharedPtr<SAudioSlider> VolumeSlider;

	float GetVolumeSliderValue() const
	{
		return Pin->GainValue;
	}

	void UpdateVolumeSliderValue(float NewValue)
	{
		Pin->GainValue = NewValue;

		MixerVertex->UpdateMuteAndSoloStates();
	};

	void UpdateMuteCheckBoxState(ECheckBoxState NewState)
	{
		Pin->bMute = NewState == ECheckBoxState::Checked;

		MixerVertex->UpdateMuteAndSoloStates();
	};

	void UpdateSoloCheckBoxState(ECheckBoxState NewState)
	{
		Pin->bSolo = NewState == ECheckBoxState::Checked;

		MixerVertex->UpdateMuteAndSoloStates();
	};

	ECheckBoxState GetMuteCheckBoxState() const
	{
		return Pin->bMute ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	};

	ECheckBoxState GetSoloCheckBoxState() const
	{
		return Pin->bSolo ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	};

	TSharedPtr<SCheckBox> MuteCheckBox;
	TSharedPtr<SCheckBox> SoloCheckBox;
};

/**
 *
 */
class BKMUSICWIDGETS_API SVariMixerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVariMixerWidget)
		{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UM2VariMixerVertex* InMixerVertex);

	UM2VariMixerVertex* MixerVertex;

	TSharedPtr<SHorizontalBox> MainHorizontalBox;

	TArray<TSharedPtr<SMixerChannelWidget>> ChannelWidgets;

	void AddChannelWidget(int ChannelIndex)
	{
		TSharedPtr<SMixerChannelWidget> NewChannelWidget;

		MainHorizontalBox->AddSlot()
			[
				SAssignNew(NewChannelWidget, SMixerChannelWidget, MixerVertex, ChannelIndex)
			];

		ChannelWidgets.Add(NewChannelWidget);
	}

	void AddChannelWidget(UM2AudioTrackPin* InPin)
	{
		UE_LOG(LogTemp, Warning, TEXT("Adding channel widget"));

		TSharedPtr<SMixerChannelWidget> NewChannelWidget;

		MainHorizontalBox->AddSlot()
			[
				SAssignNew(NewChannelWidget, SMixerChannelWidget, InPin)
			];

		ChannelWidgets.Add(NewChannelWidget);
	}
};
