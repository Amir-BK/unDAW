// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorSlateWidgets/SM2AudioOutputNode.h"
#include "SlateOptMacros.h"
#include "SAudioSlider.h"
#include "M2SoundEdGraphNodeBaseTypes.h"
#include "SAudioRadialSlider.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SM2AudioOutputNode::Construct(const FArguments& InArgs, UEdGraphNode* InGraphNode)
{
	GraphNode = InGraphNode;
	OutputNode = Cast<UM2SoundGraphAudioOutputNode>(GraphNode);
	SetTrackColorAttribute(InArgs._TrackColor);
	//bEnabledAttributesUpdate = true;

	UpdateGraphNode();
}
TSharedRef<SWidget> SM2AudioOutputNode::CreateNodeContentArea()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)

		.Padding(FMargin(0, 3))
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.FillWidth(1.0f)
				[
					// LEFT
					SAssignNew(LeftNodeBox, SVerticalBox)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoWidth()

				[
					// Center
					SAssignNew(MainVerticalBox, SVerticalBox)
						+ SVerticalBox::Slot()

						//patch select

						.AutoHeight()
						[
							//make FAudioRadialSliderStyle

							SNew(SAudioRadialSlider)
								.SliderProgressColor_Lambda([&]() {return GetSliderProgressColor(); })
								.SliderValue(OutputNode->Gain)
								.OnValueChanged_Lambda([&](float NewValue) {OutputNode->SetOutputGain(NewValue); })

						]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				[
					// RIGHT
					SAssignNew(RightNodeBox, SVerticalBox)
				]
		];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

/** Constructs this widget with InArgs */

void SM2VariMixerNode::Construct(const FArguments& InArgs, UEdGraphNode* InGraphNode)
{
	GraphNode = InGraphNode;

	UpdateGraphNode();
}

TSharedRef<SWidget> SM2VariMixerNode::CreateNodeContentArea()
{
	UM2SoundVariMixerNode* MixerNode = Cast<UM2SoundVariMixerNode>(GraphNode);
	UM2VariMixerVertex* MixerVertex = Cast<UM2VariMixerVertex>(MixerNode->Vertex);

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)

		.Padding(FMargin(0, 3))
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.FillWidth(1.0f)
				[
					// LEFT
					SAssignNew(LeftNodeBox, SVerticalBox)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoWidth()

				[
					// Center
					SAssignNew(MainVerticalBox, SVerticalBox)
						+ SVerticalBox::Slot()

						//patch select

						.AutoHeight()
						[
							//make FAudioRadialSliderStyle

							SAssignNew(MixerWidget, SVariMixerWidget, MixerVertex)

						]

				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				[
					// RIGHT
					SAssignNew(RightNodeBox, SVerticalBox)
				]
		];
}

void SM2VariMixerNode::UpdateGraphNode()
{
	SGraphNode::UpdateGraphNode();

	UM2SoundVariMixerNode* MixerNode = Cast<UM2SoundVariMixerNode>(GraphNode);
	UM2VariMixerVertex* MixerVertex = Cast<UM2VariMixerVertex>(MixerNode->Vertex);

	// add a channel widget for each input pin

	for (const auto& Pin : GraphNode->Pins)
	{
		if (Pin->Direction == EEdGraphPinDirection::EGPD_Input)
		{
			//find vertex owning the connection, safe for now
			if (Pin->HasAnyConnections())
			{
				//cast pin subcategory object to our audio pin type
				auto AsM2AudioTrackPin = Cast<UM2AudioTrackPin>(Pin->PinType.PinSubCategoryObject);

				//UEdGraphPin* LinkedPin = Pin->LinkedTo[0];

				MixerWidget->AddChannelWidget(AsM2AudioTrackPin->ChannelIndex);
				//continue;
			}

			//MixerWidget->AddChannelWidget(MixerVertex, Pin->PinName);
		}
	}

	//UpdateAudioKnobs();
}