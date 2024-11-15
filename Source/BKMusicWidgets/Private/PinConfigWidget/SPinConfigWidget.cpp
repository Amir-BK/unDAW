// Fill out your copyright notice in the Description page of Project Settings.


#include "PinConfigWidget/SPinConfigWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Pins/M2Pins.h"
#include "Vertexes/M2SoundVertex.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SComboBox.h"
#include "unDAWSettings.h"
#include "UndawWidgetsSettings.h"
#include "AudioMaterialSlate/SAudioMaterialLabeledKnob.h"
#include "AudioMaterialSlate/SAudioMaterialLabeledSlider.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPinConfigWidget::Construct(const FArguments& InArgs, const UM2Pins* InPin)
{

	Pin = InPin;

	ChildSlot
		[
			SAssignNew(MainCotentArea, SVerticalBox)
		];

	//if literal
	if (auto* AsLiteralPin = Cast<UM2MetasoundLiteralPin>(InPin))
	{
		MainCotentArea->AddSlot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("Literal Pin: %s"), *AsLiteralPin->Name.ToString())))
			];

		//if float, show min max
		if (AsLiteralPin->DataType == "Float")
		{
			
			auto* Settings = GetMutableDefault<UUNDAWSettings>();
			auto* WidgetSettings = GetDefault<UndawWidgetsSettings>();
			auto* VertexFromPin = Cast<UM2SoundVertex>(AsLiteralPin->ParentVertex);
			FName VertexName = FName(VertexFromPin->GetName());

			if (!Settings->Cache.Contains(VertexName))
			{
				FCachedVertexPinInfo NewInfo;
				Settings->Cache.Add(VertexName, NewInfo);
			}

			//if we're float pin, check if config contains reference to us, otherwise create new

			if (!Settings->Cache[VertexName].FloatPinConfigs.Contains(AsLiteralPin->Name))
			{
				FM2SoundFloatPinConfig NewConfig;
				NewConfig.MinValue = 0.0f;
				NewConfig.MaxValue = 1.0f;
				NewConfig.KnobStyleOverride = WidgetSettings->KnobStyleOverride;
				NewConfig.SliderStyleOverride = WidgetSettings->SliderStyleOverride;
				Settings->Cache[VertexName].FloatPinConfigs.Add(AsLiteralPin->Name, NewConfig);
			}

			//knob style option source
			
			WidgetTypes.Add(MakeShareable(new FString("Knob")));
			WidgetTypes.Add(MakeShareable(new FString("Slider")));
			WidgetTypes.Add(MakeShareable(new FString("None")));

			//linear/volume/frequency enum
			MainCotentArea->AddSlot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Widget Type:"))))

						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SComboBox<TSharedPtr<FString>>)
								.OptionsSource(&WidgetTypes)
								.OnGenerateWidget(this, &SPinConfigWidget::OnGenerateValueTypeEnumWidget)
								.InitiallySelectedItem(GetCurentWidgetType())
								.OnSelectionChanged_Lambda([this, Settings, AsLiteralPin, WidgetSettings, VertexName](TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo)
									{
										if (InItem->Equals("Knob"))
										{
											//->Cache[VertexName].FloatPinConfigs[AsLiteralPin->Name].KnobStyleOverride = WidgetSettings->KnobStyleOverride;
											Settings->Cache[VertexName].FloatPinConfigs[AsLiteralPin->Name].WidgetType = EFloatPinWidgetType::Knob;
										}
										else if (InItem->Equals("Slider"))
										{
											//Settings->Cache[VertexName].FloatPinConfigs[AsLiteralPin->Name].KnobStyleOverride = WidgetSettings->SliderStyleOverride;
											Settings->Cache[VertexName].FloatPinConfigs[AsLiteralPin->Name].WidgetType = EFloatPinWidgetType::Slider;
										}
										else if (InItem->Equals("None"))
										{
											Settings->Cache[VertexName].FloatPinConfigs[AsLiteralPin->Name].KnobStyleOverride = FSoftObjectPath();
											Settings->Cache[VertexName].FloatPinConfigs[AsLiteralPin->Name].WidgetType = EFloatPinWidgetType::NoWidget;

										}

										Settings->SaveConfig();
									})
								[
									SNew(STextBlock)
										.Text_Lambda([this]() { return FText::FromString(*GetCurentWidgetType()); })
								]

						]

				];

			//unit type combobox
			UnitTypes.Add(MakeShareable(new FString("Linear")));
			UnitTypes.Add(MakeShareable(new FString("Volume")));
			UnitTypes.Add(MakeShareable(new FString("Frequency")));

			MainCotentArea->AddSlot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Unit Type:"))))

						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SComboBox<TSharedPtr<FString>>)
								.OptionsSource(&UnitTypes)
								.OnGenerateWidget(this, &SPinConfigWidget::OnGenerateUnitTypeEnumWidget)
								.InitiallySelectedItem(GetCurrentUnitType())
								.OnSelectionChanged_Lambda([this, Settings, AsLiteralPin, VertexName](TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo)
									{
										if (InItem->Equals("Linear"))
										{
											Settings->Cache[VertexName].FloatPinConfigs[AsLiteralPin->Name].UnitType = EAudioUnitsValueType::Linear;
										}
										else if (InItem->Equals("Volume"))
										{
											Settings->Cache[VertexName].FloatPinConfigs[AsLiteralPin->Name].UnitType = EAudioUnitsValueType::Volume;
										}
										else if (InItem->Equals("Frequency"))
										{
											Settings->Cache[VertexName].FloatPinConfigs[AsLiteralPin->Name].UnitType = EAudioUnitsValueType::Frequency;
										}

										Settings->SaveConfig();
									})
								[
									SNew(STextBlock)
										.Text_Lambda([this]() { return FText::FromString(*GetCurrentUnitType()); })
								]

						]

				];


			MainCotentArea->AddSlot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Min:"))))
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<float>)
								.Value_Lambda([this]() {return MinValue; })
								.OnValueChanged_Lambda([this](float InValue) {MinValue = InValue;
								UpdateMinMax(MinValue, MaxValue); })
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Max:"))))
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<float>)
								.Value_Lambda([this]() {return MaxValue; })
								.OnValueChanged_Lambda([this](float InValue) {MaxValue = InValue;
							UpdateMinMax(MinValue, MaxValue); })
						]

				];
		}

		if (AsLiteralPin->DataType == "Int32")
			{
			MainCotentArea->AddSlot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Min:"))))
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<int32>)
								.Value(0)
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Max:"))))
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<int32>)
								.Value(1)
						]

				];
		}


	}

}

void SPinConfigWidget::UpdateMinMax(float InMin, float InMax)
{
	MinValue = InMin;
	MaxValue = InMax;

	FName SearchName = FName(Pin->ParentVertex->GetName());
	const auto& Settings = UUNDAWSettings::Get();
	Settings->Cache.Contains(SearchName);
}

inline TSharedRef<SWidget> SPinConfigWidget::OnGenerateValueTypeEnumWidget(TSharedPtr<FString> InItem)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InItem));

}

TSharedRef<SWidget> SPinConfigWidget::OnGenerateUnitTypeEnumWidget(TSharedPtr<FString> InItem)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InItem));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

