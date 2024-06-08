// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "M2SoundEdGraphSchema.h"

/**
 * 
 */
class BK_EDITORUTILITIES_API SM2AudioOutputNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SM2AudioOutputNode)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UEdGraphNode* InGraphNode);

	TSharedRef<SWidget> CreateNodeContentArea() override;


	UM2SoundGraphAudioOutputNode* OutputNode;

	TSharedPtr<SVerticalBox> MainVerticalBox;

	FLinearColor GetSliderProgressColor() const
	{
		return OutputNode->GetNodeTitleColor()	;
	}
};
