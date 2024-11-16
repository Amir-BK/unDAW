// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pins/M2Pins.h"
#include "Vertexes/M2SoundVertex.h"
#include "Widgets/SBoxPanel.h"
#include "unDAWSettings.h"
#include "M2SoundGraphStatics.h"
#include "Styling/SlateWidgetStyleAsset.h"
#include "Widgets/SCompoundWidget.h"



/**
 * 
 */
class BKMUSICWIDGETS_API SPinConfigWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPinConfigWidget)
	{}
		SLATE_EVENT(FSimpleDelegate, OnConfigChanged)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const UM2Pins* InPin);

	float MinValue = 0.0f;

	float MaxValue = 1.0f;

	FName PatchName = NAME_None;

	//the laziest...
	TArray<TSharedPtr<FString>> WidgetTypes;

	TArray<TSharedPtr<FString>> OritentationTypes;

	TArray<TSharedPtr<FString>> UnitTypes;

	TArray<TSharedPtr<FString>> StylesStrings;

	FSimpleDelegate OnConfigChanged;

	void UpdateMinMax(float InMin, float InMax);

	TArray<UObject*> KnobStyleObjects;
	TArray<UObject*> SliderStyleObjects;

	const UM2Pins* Pin;

	TSharedPtr<SVerticalBox> MainCotentArea;

	TArray<TSharedPtr<FString>> GetAllKnobStyles()
	{
		TArray<TSharedPtr<FString>> Styles;
		Styles.Add(MakeShareable(new FString("Default")));

		KnobStyleObjects = UM2SoundGraphStatics::GetAllObjectsOfClass(USlateWidgetStyleAsset::StaticClass());

		for (auto* Style : KnobStyleObjects)
		{
			Styles.Add(MakeShareable(new FString(Style->GetName())));
		}

		return Styles;
	}


	TSharedPtr<FString> GetCurentWidgetType() const
	{
		//this is only called for float so, get value from settings
		auto* Settings = GetDefault<UUNDAWSettings>();

		if (Settings->Cache.Contains(PatchName))
		{
			if (Settings->Cache[PatchName].FloatPinConfigs.Contains(Pin->Name))
			{
				auto& Config = Settings->Cache[PatchName].FloatPinConfigs[Pin->Name];
				if (Config.WidgetType == EFloatPinWidgetType::Slider)
				{
					return WidgetTypes[1];
				}
				else if (Config.WidgetType == EFloatPinWidgetType::Knob)
				{
					return WidgetTypes[0];
				}
			}
		}


		return WidgetTypes[2];
	};

	TSharedPtr<FString> GetCurrentUnitType() const
	{
		auto* Settings = GetDefault<UUNDAWSettings>();

		//there are only three types...
		if (Settings->Cache.Contains(PatchName))
		{
			if (Settings->Cache[PatchName].FloatPinConfigs.Contains(Pin->Name))
			{
				auto& Config = Settings->Cache[PatchName].FloatPinConfigs[Pin->Name];
				if (Config.UnitType == EAudioUnitsValueType::Linear)
				{
					return UnitTypes[0];
				}
				else if (Config.UnitType == EAudioUnitsValueType::Volume)
				{
					return UnitTypes[1];
				}
				else if (Config.UnitType == EAudioUnitsValueType::Frequency)
				{
					return UnitTypes[2];
				}
			}
		}

		return UnitTypes[0];



	}

	TSharedPtr<FString> GetCurrentOrientation()
	{

		auto* Settings = GetDefault<UUNDAWSettings>();

		if (Settings->Cache.Contains(PatchName))
		{
			if (Settings->Cache[PatchName].FloatPinConfigs.Contains(Pin->Name))
			{
				auto& Config = Settings->Cache[PatchName].FloatPinConfigs[Pin->Name];
				if (Config.SliderOrientation == EOrientation::Orient_Horizontal)
				{
					return OritentationTypes[0];
				}
				else if (Config.SliderOrientation == EOrientation::Orient_Vertical)
				{
					return OritentationTypes[1];
				}
			}
		}

		return OritentationTypes[0];
	}


	TSharedRef<SWidget> OnGenerateValueTypeEnumWidget(TSharedPtr<FString> InItem);

	TSharedRef<SWidget> OnGenerateUnitTypeEnumWidget(TSharedPtr<FString> InItem);
};
