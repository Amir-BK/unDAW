// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorSlateWidgets/SM2MidiTrackGraphNode.h"
#include "SlateOptMacros.h"
#include "SAudioSlider.h"
#include "Widgets/Input/SCheckBox.h"

#include "SAudioRadialSlider.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SM2MidiTrackGraphNode::Construct(const FArguments& InArgs, UEdGraphNode* InGraphNode)
{
	GraphNode = InGraphNode;
	InputNode = Cast<UM2SoundGraphInputNode>(InGraphNode);
	SetTrackColorAttribute(InArgs._TrackColor);
	//bEnabledAttributesUpdate = true;

	UpdateGraphNode();
}
TSharedRef<SWidget> SM2MidiTrackGraphNode::CreateNodeContentArea()
{
	UM2SoundBuilderInputHandleVertex* AsInputVertex = Cast< UM2SoundBuilderInputHandleVertex>(InputNode->Vertex);

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

							SNew(SCheckBox)
								.IsChecked_Lambda([AsInputVertex] {return AsInputVertex->bOutputToBlueprints ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
								.OnCheckStateChanged_Lambda([AsInputVertex](ECheckBoxState NewState) {AsInputVertex->bOutputToBlueprints = NewState == ECheckBoxState::Checked; })
								[
									SNew(STextBlock)
										.Text(FText::FromString("Graph Output"))
										.ToolTipText(FText::FromString("When checked the MIDI output, after having been filtered by track/ch\nwill be assigned to a metasound graph output\nwhere it can be used to be connected to metasound watch outputs"))
								]

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