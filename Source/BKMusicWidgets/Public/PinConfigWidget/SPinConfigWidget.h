// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pins/M2Pins.h"
#include "Vertexes/M2SoundVertex.h"
#include "Widgets/SBoxPanel.h"
#include "unDAWSettings.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class BKMUSICWIDGETS_API SPinConfigWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPinConfigWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const UM2Pins* InPin);

	float MinValue = 0.0f;

	float MaxValue = 1.0f;

	//the laziest...
	TArray<TSharedPtr<FString>> WidgetTypes;

	TArray<TSharedPtr<FString>> UnitTypes;

	void UpdateMinMax(float InMin, float InMax);

	const UM2Pins* Pin;

	TSharedPtr<SVerticalBox> MainCotentArea;

	TSharedPtr<FString> GetCurentWidgetType() const
	{
		//this is only called for float so, get value from settings
		auto* Settings = GetDefault<UUNDAWSettings>();

		if (Settings->Cache.Contains(Pin->ParentVertex->GetFName()))
		{
			if (Settings->Cache[Pin->ParentVertex->GetFName()].FloatPinConfigs.Contains(Pin->Name))
			{
				auto& Config = Settings->Cache[Pin->ParentVertex->GetFName()].FloatPinConfigs[Pin->Name];
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
		if (Settings->Cache.Contains(Pin->ParentVertex->GetFName()))
		{
			if (Settings->Cache[Pin->ParentVertex->GetFName()].FloatPinConfigs.Contains(Pin->Name))
			{
				auto& Config = Settings->Cache[Pin->ParentVertex->GetFName()].FloatPinConfigs[Pin->Name];
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

	TSharedRef<SWidget> OnGenerateValueTypeEnumWidget(TSharedPtr<FString> InItem);

	TSharedRef<SWidget> OnGenerateUnitTypeEnumWidget(TSharedPtr<FString> InItem);
};
