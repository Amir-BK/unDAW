// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "M2SoundEdGraphSchema.h"
#include "SAudioRadialSlider.h"
#include "SAudioSlider.h"
#include "M2SoundEdGraphNodeBaseTypes.h"
#include <AutoPatchWidgets/SVariMixerWidget.h>

/**
 *
 */
class BK_EDITORUTILITIES_API SM2AudioOutputNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SM2AudioOutputNode)
		{	}
		SLATE_ATTRIBUTE(FLinearColor, TrackColor)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UEdGraphNode* InGraphNode);

	TSharedRef<SWidget> CreateNodeContentArea() override;

	TAttribute<FLinearColor> TrackColor;

	void SetTrackColorAttribute(TAttribute<FLinearColor> InTrackColor)
	{
		TrackColor = InTrackColor;
	}

	UM2SoundGraphAudioOutputNode* OutputNode;

	TSharedPtr<SVerticalBox> MainVerticalBox;

	TSharedPtr<SAudioRadialSlider> RadialSlider;

	FLinearColor GetSliderProgressColor() const
	{
		return OutputNode->GetNodeTitleColor();
	}
};

class BK_EDITORUTILITIES_API SM2VariMixerNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SM2VariMixerNode)
		{	}
		SLATE_ATTRIBUTE(FLinearColor, TrackColor)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UEdGraphNode* InGraphNode);

	TSharedRef<SWidget> CreateNodeContentArea() override;

	void UpdateGraphNode() override;

	
	TSharedPtr<SVariMixerWidget> MixerWidget;

	TArray<TSharedPtr<SAudioSlider>> Sliders;

	TSharedPtr<SVerticalBox> MainVerticalBox;
};