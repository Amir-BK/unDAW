// Fill out your copyright notice in the Description page of Project Settings.

#include "AutoPatchWidgets/SVariMixerWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SVariMixerWidget::Construct(const FArguments& InArgs, UM2VariMixerVertex* InMixerVertex)
{
	MixerVertex = InMixerVertex;

	ChildSlot
		[
			SAssignNew(MainHorizontalBox, SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SMixerChannelWidget, InMixerVertex, INDEX_NONE)
						.IsEnabled(false)
						.Visibility_Lambda([this]() -> EVisibility { return ChannelWidgets.Num() > 0 ? EVisibility::Collapsed : EVisibility::Visible; })
				]

		];
}

void SMixerChannelWidget::Construct(const FArguments& InArgs, UM2VariMixerVertex* InMixerVertex, uint8 InChannelIndex)
{
	MixerVertex = InMixerVertex;
	ChannelIndex = InChannelIndex;

	ChildSlot
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)

				[

					SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("Channel %d"), ChannelIndex)))
				]

				+ SVerticalBox::Slot()
				.MaxHeight(175)
				.Padding(5)
				[
					SAssignNew(VolumeSlider, SAudioSlider)
						.SliderBackgroundColor_Lambda([this]() -> FLinearColor { return MixerVertex->GetChannelColor(ChannelIndex); })
						.SliderThumbColor_Lambda([this]() -> FLinearColor { return MixerVertex->GetChannelColor(ChannelIndex); })
						.SliderValue(this, &SMixerChannelWidget::GetVolumeSliderValue)
						.OnValueChanged_Lambda([this](float NewValue) { MixerVertex->UpdateGainParameter(ChannelIndex, NewValue); })
				]
				//+ SVerticalBox::Slot()
				//[
				//	SAssignNew(RadialSlider, SAudioRadialSlider)

				//		//.Value(this, &SMixerChannelWidget::GetRadialSliderValue)
				//]

				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)

				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						[
							SNew(SBorder)
								.BorderBackgroundColor(FLinearColor::Yellow)
								.BorderImage(FCoreStyle::Get().GetBrush("Menu.Background"))

								[
									SAssignNew(MuteCheckBox, SCheckBox)
										.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { MixerVertex->SetChannelMuteState(ChannelIndex, NewState); })
										.IsChecked(this, &SMixerChannelWidget::GetMuteCheckBoxState)
										.ToolTipText(FText::FromString("Mute"))
								]

								//	.IsChecked(this, &SMixerChannelWidget::GetMuteCheckBoxState)
						]

						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)

						[
							SNew(SBorder)
								.BorderBackgroundColor(FLinearColor::Green)
								.BorderImage(FCoreStyle::Get().GetBrush("Menu.Background"))

								[
									SAssignNew(SoloCheckBox, SCheckBox)
										.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { MixerVertex->SetChannelSoloState(ChannelIndex, NewState); })
										.IsChecked(this, &SMixerChannelWidget::GetSoloCheckBoxState)
										.ToolTipText(FText::FromString("Solo"))

								]

								//	.IsChecked(this, &SMixerChannelWidget::GetSoloCheckBoxState)
						]

				]

		];

	//RadialSlider->SetOutputRange(FVector2D(-1, 1));
}

void SMixerChannelWidget::Construct(const FArguments& InArgs, UM2AudioTrackPin* InPin)
{
	Pin = InPin;
	
	ChildSlot
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)

				[

					SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("Channel %d"), ChannelIndex)))
				]

				+ SVerticalBox::Slot()
				.MaxHeight(175)
				.Padding(5)
				[
					SAssignNew(VolumeSlider, SAudioSlider)
					//	.SliderBackgroundColor_Lambda([this]() -> FLinearColor { return MixerVertex->GetChannelColor(ChannelIndex); })
					//	.SliderThumbColor_Lambda([this]() -> FLinearColor { return MixerVertex->GetChannelColor(ChannelIndex); })
					//	.SliderValue(this, &SMixerChannelWidget::GetVolumeSliderValue)
					//	.OnValueChanged_Lambda([this](float NewValue) { MixerVertex->UpdateGainParameter(ChannelIndex, NewValue); })
				]
				//+ SVerticalBox::Slot()
				//[
				//	SAssignNew(RadialSlider, SAudioRadialSlider)

				//		//.Value(this, &SMixerChannelWidget::GetRadialSliderValue)
				//]

				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)

				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						[
							SNew(SBorder)
								.BorderBackgroundColor(FLinearColor::Yellow)
								.BorderImage(FCoreStyle::Get().GetBrush("Menu.Background"))

								[
									SAssignNew(MuteCheckBox, SCheckBox)
									//	.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { MixerVertex->SetChannelMuteState(ChannelIndex, NewState); })
									//	.IsChecked(this, &SMixerChannelWidget::GetMuteCheckBoxState)
										.ToolTipText(FText::FromString("Mute"))
								]

								//	.IsChecked(this, &SMixerChannelWidget::GetMuteCheckBoxState)
						]

						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)

						[
							SNew(SBorder)
								.BorderBackgroundColor(FLinearColor::Green)
								.BorderImage(FCoreStyle::Get().GetBrush("Menu.Background"))

								[
									SAssignNew(SoloCheckBox, SCheckBox)
										.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { MixerVertex->SetChannelSoloState(ChannelIndex, NewState); })
										//.IsChecked(this, &SMixerChannelWidget::GetSoloCheckBoxState)
										.ToolTipText(FText::FromString("Solo"))

								]

								//	.IsChecked(this, &SMixerChannelWidget::GetSoloCheckBoxState)
						]

				]

		];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION