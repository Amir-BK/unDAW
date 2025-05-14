// Fill out your copyright notice in the Description page of Project Settings.


#include "PinConfigWidget/SPinConfigWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Pins/M2Pins.h"
#include "Vertexes/M2SoundVertex.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SComboBox.h"
#include "SSearchableComboBox.h"
#include "unDAWSettings.h"
#include "UndawWidgetsSettings.h"
#include "AudioMaterialSlate/SAudioMaterialLabeledKnob.h"
#include "AudioMaterialSlate/SAudioMaterialLabeledSlider.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPinConfigWidget::Construct(const FArguments& InArgs, const UM2Pins* InPin)
{
	auto* Settings = GetMutableDefault<UUNDAWSettings>();
	auto* WidgetSettings = GetDefault<UndawWidgetsSettings>();
	Pin = InPin;
	//cast parent to patch vertex and get name
	OnConfigChanged = InArgs._OnConfigChanged;
	auto* VertexFromPin = Cast<UM2SoundPatch>(InPin->ParentVertex);
	PatchName = VertexFromPin->Patch->GetFName();

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
		if (AsLiteralPin->DataType == "Float" || AsLiteralPin->DataType == "Bool")
		{
			

			//auto* VertexFromPin = Cast<UM2SoundPatch>(AsLiteralPin->ParentVertex);
			//FName VertexName = VertexFromPin->Patch->GetFName();

			if (!Settings->Cache.Contains(PatchName))
			{
				FCachedVertexPinInfo NewInfo;
				Settings->Cache.Add(PatchName, NewInfo);
			}

			//if we're float pin, check if config contains reference to us, otherwise create new

			if (!Settings->Cache[PatchName].FloatPinConfigs.Contains(AsLiteralPin->Name))
			{
				FM2SoundFloatPinConfig NewConfig;
				NewConfig.MinValue = 0.0f;
				NewConfig.MaxValue = 1.0f;
				NewConfig.KnobStyleOverride = WidgetSettings->KnobStyleOverride;
				NewConfig.SliderStyleOverride = WidgetSettings->SliderStyleOverride;
				Settings->Cache[PatchName].FloatPinConfigs.Add(AsLiteralPin->Name, NewConfig);
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
								.OnSelectionChanged_Lambda([this, Settings, AsLiteralPin, WidgetSettings](TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo)
									{
										if (InItem->Equals("Knob"))
										{
											//->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].KnobStyleOverride = WidgetSettings->KnobStyleOverride;
											Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].WidgetType = EFloatPinWidgetType::Knob;
										}
										else if (InItem->Equals("Slider"))
										{
											//Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].KnobStyleOverride = WidgetSettings->SliderStyleOverride;
											Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].WidgetType = EFloatPinWidgetType::Slider;
										}
										else if (InItem->Equals("None"))
										{
											Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].KnobStyleOverride = FSoftObjectPath();
											Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].WidgetType = EFloatPinWidgetType::NoWidget;

										}

										Settings->SaveConfig();

										OnConfigChanged.ExecuteIfBound();
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
								.OnSelectionChanged_Lambda([this, Settings, AsLiteralPin](TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo)
									{
										if (InItem->Equals("Linear"))
										{
											Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].UnitType = EAudioUnitsValueType::Linear;
										}
										else if (InItem->Equals("Volume"))
										{
											Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].UnitType = EAudioUnitsValueType::Volume;
										}
										else if (InItem->Equals("Frequency"))
										{
											Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].UnitType = EAudioUnitsValueType::Frequency;
										}

										Settings->SaveConfig();

										OnConfigChanged.ExecuteIfBound();
									})
								[
									SNew(STextBlock)
										.Text_Lambda([this]() { return FText::FromString(*GetCurrentUnitType()); })
								]

						]

				];

			StylesStrings = GetAllKnobStyles();

			//style selection
			MainCotentArea->AddSlot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Knob Style:"))))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SSearchableComboBox)
								.OptionsSource(&StylesStrings)
								.OnGenerateWidget(this, &SPinConfigWidget::OnGenerateUnitTypeEnumWidget)
								//.InitiallySelectedItem(GetCurrentStyle())
								.OnSelectionChanged_Lambda([this, Settings, AsLiteralPin](TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo)
									{
										for (const auto& KnobStyleObject : KnobStyleObjects)
										{
											if (KnobStyleObject->GetName().Equals(*InItem))
											{
												Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].KnobStyleOverride = FSoftObjectPath(KnobStyleObject);
												Settings->SaveConfig();
												OnConfigChanged.ExecuteIfBound();
											}

										}


									})
								[
									SNew(STextBlock)
										.Text_Lambda([this]() { return FText::FromString("Test"); })
								]
						]
						
				];

			//now the same for slider styles

			MainCotentArea->AddSlot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Slider Style:"))))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SSearchableComboBox)
								.OptionsSource(&StylesStrings)
								.OnGenerateWidget(this, &SPinConfigWidget::OnGenerateUnitTypeEnumWidget)
								//.InitiallySelectedItem(GetCurrentStyle())
								.OnSelectionChanged_Lambda([this, Settings, AsLiteralPin](TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo)
									{
										for (const auto& SliderStyleObject : KnobStyleObjects)
										{
											if (SliderStyleObject->GetName().Equals(*InItem))
											{
												Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].SliderStyleOverride = FSoftObjectPath(SliderStyleObject);
												Settings->SaveConfig();
												OnConfigChanged.ExecuteIfBound();
											}

										}
									})
								[
									SNew(STextBlock)
										.Text_Lambda([this]() { return FText::FromString("Test"); })
								]
						]
				];

			OritentationTypes.Add(MakeShareable(new FString("Horizontal")));
			OritentationTypes.Add(MakeShareable(new FString("Vertical")));

			//slider orientation
			MainCotentArea->AddSlot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Slider Orientation:"))))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SComboBox<TSharedPtr<FString>>)
								.OptionsSource(&OritentationTypes)
								.OnGenerateWidget(this, &SPinConfigWidget::OnGenerateValueTypeEnumWidget)
							//	.InitiallySelectedItem(GetCurrentOrientation())
								.OnSelectionChanged_Lambda([this, Settings, AsLiteralPin](TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo)
									{
										if (InItem->Equals("Horizontal"))
										{
											Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].SliderOrientation = EOrientation::Orient_Horizontal;
										}
										else if (InItem->Equals("Vertical"))
										{
											Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].SliderOrientation = EOrientation::Orient_Vertical;
										}

										Settings->SaveConfig();

										OnConfigChanged.ExecuteIfBound();
									})
								[
									SNew(STextBlock)
										.Text_Lambda([this]() { return FText::FromString(*GetCurrentOrientation()); })
								]
						]
				];

			MainCotentArea->AddSlot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(STextBlock)
						.Text(INVTEXT("Grid Position"))
				];

			//get GridX and GridY from settings

			MainCotentArea->AddSlot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Grid X:"))))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<int32>)
								.AllowSpin(true)
								.Value_Lambda([this, Settings, AsLiteralPin]() {return Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].GridX; })
								.OnValueChanged_Lambda([this, Settings, AsLiteralPin](int32 InValue) {
								Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].GridX = InValue;
								Settings->SaveConfig();

								OnConfigChanged.ExecuteIfBound(); })
						]
						//Y...
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Grid Y:"))))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<int32>)
								.AllowSpin(true)
								.Value_Lambda([this, Settings, AsLiteralPin]() {return Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].GridY; })
								.OnValueChanged_Lambda([this, Settings, AsLiteralPin](int32 InValue) {
								Settings->Cache[PatchName].FloatPinConfigs[AsLiteralPin->Name].GridY = InValue;
								Settings->SaveConfig();

								OnConfigChanged.ExecuteIfBound(); })
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
	bool bContains = Settings->Cache.Contains(SearchName);
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

