// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vertexes/M2VariMixerVertex.h"
#include "M2SoundGraphData.h"
#include "SAudioRadialSlider.h"
#include "SAudioSlider.h"
#include "Widgets/SCompoundWidget.h"




class BKMUSICWIDGETS_API SMixerChannelWidget : public SCompoundWidget
{
	public:
	SLATE_BEGIN_ARGS(SMixerChannelWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UM2VariMixerVertex* InMixerVertex, uint8 InChannelIndex);

	UM2VariMixerVertex* MixerVertex;
	uint8 ChannelIndex;

	TSharedPtr<SAudioRadialSlider> RadialSlider;
	TSharedPtr<SAudioSlider> VolumeSlider;



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
};