// Fill out your copyright notice in the Description page of Project Settings.

#include "AutoPatchWidgets/SVariMixerWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"
#include "AudioMaterialSlate/SAudioMaterialLabeledSlider.h"
#include "UndawWidgetsSettings.h"
#include "Widgets/SBoxPanel.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SVariMixerWidget::Construct(const FArguments& InArgs, UM2VariMixerVertex* InMixerVertex)
{
	MixerVertex = InMixerVertex;

	ChildSlot
		[
			SAssignNew(MainHorizontalBox, SHorizontalBox)
				
		];
}

void SVariMixerWidget::AddChannelWidget(UM2AudioTrackPin* InPin)
{

	TSharedPtr<SMixerChannelWidget> NewChannelWidget;

	MainHorizontalBox->AddSlot()
		.AutoWidth()
		[
			SAssignNew(NewChannelWidget, SMixerChannelWidget, InPin)
	
		];

	ChannelWidgets.Add(NewChannelWidget);
}



void SMixerChannelWidget::Construct(const FArguments& InArgs, UM2AudioTrackPin* InPin)
{
	Pin = InPin;
	MixerVertex = Cast<UM2VariMixerVertex>(Pin->ParentVertex);

	FString LinkedToChannelName = Pin->LinkedPin ? Pin->LinkedPin->ParentVertex->GetVertexDisplayName() : FString("None");

	//settings for style
	const UndawWidgetsSettings* Settings = GetDefault<UndawWidgetsSettings>();
	
	ChildSlot
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)

				[

					SNew(SBox)
						.HeightOverride(30)
						.WidthOverride(150)
						[
							SNew(STextBlock)
								.Text(FText::FromString(LinkedToChannelName))
								.AutoWrapText(true)
								.Justification(ETextJustify::Center)
						]

				]

				//panning knob
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					

									SAssignNew(PanKnob, SAudioRadialSlider)
										.DesiredSizeOverride(FVector2D(100, 100))
										// Set the initial normalized value to 0.5, which will map to 0 pan.
										.SliderValue(0.5f)
										.OnValueChanged(this, &SMixerChannelWidget::UpdatePanKnobValue)
										// You can leave the default HandStartEndRatio or adjust if needed.
										.HandStartEndRatio(FVector2D(0.0f, 1.0f))
										

										//.OnValueChanged(this, &SMixerChannelWidget::UpdatePanKnobValue)

				]

				+ SVerticalBox::Slot()
				//.MaxHeight(175)
				//.Padding(5)
				[
					SAssignNew(VolumeLabeledSlider, SAudioMaterialLabeledSlider)
					//	.LabelText(FText::FromString("Volume"))
						.SliderValue(this, &SMixerChannelWidget::GetVolumeSliderValue)
						.OnValueChanged(this, &SMixerChannelWidget::UpdateVolumeSliderValue)
						.Style(Settings->GetSliderStyle())
						.Orientation(EOrientation::Orient_Vertical)	
						.AudioUnitsValueType(EAudioUnitsValueType::Volume)
				]

				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				.HAlign(HAlign_Center)

				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.AutoWidth()
						[
							SNew(SBorder)
								.BorderBackgroundColor(FLinearColor::Yellow)
								.BorderImage(FCoreStyle::Get().GetBrush("Menu.Background"))

								[
									SAssignNew(MuteCheckBox, SCheckBox)
										.OnCheckStateChanged(this, &SMixerChannelWidget::UpdateMuteCheckBoxState)
									//	.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { MixerVertex->SetChannelMuteState(ChannelIndex, NewState); })
										.IsChecked(this, &SMixerChannelWidget::GetMuteCheckBoxState)
										.ToolTipText(FText::FromString("Mute"))
								]

								//	.IsChecked(this, &SMixerChannelWidget::GetMuteCheckBoxState)
						]

						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.AutoWidth()

						[
							SNew(SBorder)
								.BorderBackgroundColor(FLinearColor::Green)
								.BorderImage(FCoreStyle::Get().GetBrush("Menu.Background"))

								[
									SAssignNew(SoloCheckBox, SCheckBox)
										.OnCheckStateChanged(this, &SMixerChannelWidget::UpdateSoloCheckBoxState)
										//.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { MixerVertex->SetChannelSoloState(ChannelIndex, NewState); })
										.IsChecked(this, &SMixerChannelWidget::GetSoloCheckBoxState)
										.ToolTipText(FText::FromString("Solo \n Exclusive Solo - Ctrl + Click"))

								]

								//	.IsChecked(this, &SMixerChannelWidget::GetSoloCheckBoxState)
						]

				]

		];

		PanKnob->SetOutputRange(FVector2D(-1.0f, 1.0f));

}

void SMixerChannelWidget::UpdatePanKnobValue(float NewValue)
{
	Pin->PanValue = NewValue;

	FName FloatName;
	EMetaSoundBuilderResult BuildResult;


	auto NewFloatLiteral = MixerVertex->BuilderSubsystem->CreateFloatMetaSoundLiteral(NewValue, FloatName);
	MixerVertex->BuilderContext->SetNodeInputDefault(Pin->PanParameter->GetHandle<FMetaSoundBuilderNodeInputHandle>(), NewFloatLiteral, BuildResult);

	{
		//for debugging purposes let's print all the details of the handle
		FName NodeName, NodeData;
		MixerVertex->BuilderContext->GetNodeInputData(Pin->PanParameter->GetHandle<FMetaSoundBuilderNodeInputHandle>(), NodeName, NodeData, BuildResult);

		UE_LOG(LogTemp, Warning, TEXT("NodeName: %s, NodeData: %s"), *NodeName.ToString(), *NodeData.ToString());

	}


	//print handle and result
	FString ResultToString = BuildResult == EMetaSoundBuilderResult::Succeeded ? "Succeeded" : "Failed";

	UE_LOG(LogTemp, Warning, TEXT("Result: %s"), *ResultToString);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION