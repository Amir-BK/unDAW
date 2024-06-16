// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "M2SoundEdGraphSchema.h"
#include "M2SoundEdGraphNodeBaseTypes.h"
#include "SAudioRadialSlider.h"

/**
 *
 */
class BK_EDITORUTILITIES_API SM2MidiTrackGraphNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SM2MidiTrackGraphNode)
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

	UM2SoundGraphInputNode* InputNode;

	TSharedPtr<SVerticalBox> MainVerticalBox;

	TSharedPtr<SAudioRadialSlider> RadialSlider;
};
