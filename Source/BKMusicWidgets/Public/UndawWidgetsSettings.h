// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AudioWidgetsStyle.h"
#include "Engine/DeveloperSettings.h"
#include "AudioMaterialSlate/AudioMaterialSlateTypes.h"
#include "AudioWidgetsStyle.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateWidgetStyleAsset.h"

//autogen
#include "UndawWidgetsSettings.generated.h"

/**
 * 
 */
UCLASS(config = Game, meta = (DisplayName = "unDAW Widgets Settings"))
class BKMUSICWIDGETS_API UndawWidgetsSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UndawWidgetsSettings();
	~UndawWidgetsSettings();


	/**Override the Knob Style used in the Metasound Editor.*/
	UPROPERTY(EditAnywhere, config, Category = "Widget Styling (Experimental)", meta = (AllowedClasses = "/Script/SlateCore.SlateWidgetStyleAsset", EditCondition = "bUseAudioMaterialWidgets", DisplayName = "Knob Style"))
	FSoftObjectPath KnobStyleOverride;

	/**Override the Slider Style used in the Metasound Editor.*/
	UPROPERTY(EditAnywhere, config, Category = "Widget Styling (Experimental)", meta = (AllowedClasses = "/Script/SlateCore.SlateWidgetStyleAsset", EditCondition = "bUseAudioMaterialWidgets", DisplayName = "Slider Style"))
	FSoftObjectPath SliderStyleOverride;

	/**Override the Button Style used in the Metasound Editor.*/
	UPROPERTY(EditAnywhere, config, Category = "Widget Styling (Experimental)", meta = (AllowedClasses = "/Script/SlateCore.SlateWidgetStyleAsset", EditCondition = "bUseAudioMaterialWidgets", DisplayName = "Button Style"))
	FSoftObjectPath ButtonStyleOverride;

	/**Override the Meter Style used in the Metasound Editor.*/
	UPROPERTY(EditAnywhere, config, Category = "Widget Styling (Experimental)", meta = (AllowedClasses = "/Script/SlateCore.SlateWidgetStyleAsset", EditCondition = "bUseAudioMaterialWidgets", DisplayName = "Meter Style"))
	FSoftObjectPath MeterStyleOverride;

	/** Get the AudioMaterialKnob Style. If KnobStyleOverride is not set, returns default style.*/
	const FAudioMaterialKnobStyle* GetKnobStyle() const
	{
		if (const UObject* Style = KnobStyleOverride.TryLoad())
		{
			if (const USlateWidgetStyleAsset* SlateWidgetStyleAsset = CastChecked<USlateWidgetStyleAsset>(Style))
			{
				return SlateWidgetStyleAsset->GetStyle<FAudioMaterialKnobStyle>();
			}
		}

		return &FAudioWidgetsStyle::Get().GetWidgetStyle<FAudioMaterialKnobStyle>("AudioMaterialKnob.Style");
	}

	/** Get the AudioMaterialSlider Style. If SliderStyleOverride is not set, returns default style.*/
	const FAudioMaterialSliderStyle* GetSliderStyle() const
	{
		if (const UObject* Style = SliderStyleOverride.TryLoad())
		{
			if (const USlateWidgetStyleAsset* SlateWidgetStyleAsset = CastChecked<USlateWidgetStyleAsset>(Style))
			{
				return SlateWidgetStyleAsset->GetStyle<FAudioMaterialSliderStyle>();
			}
		}

		return &FAudioWidgetsStyle::Get().GetWidgetStyle<FAudioMaterialSliderStyle>("AudioMaterialSlider.Style");
	}

	/** Get the AudioMaterialButton Style. If ButtonStyleOverride is not set, returns default style.*/
	const FAudioMaterialButtonStyle* GetButtonStyle() const
	{
		if (const UObject* Style = ButtonStyleOverride.TryLoad())
		{
			if (const USlateWidgetStyleAsset* SlateWidgetStyleAsset = CastChecked<USlateWidgetStyleAsset>(Style))
			{
				return SlateWidgetStyleAsset->GetStyle<FAudioMaterialButtonStyle>();
			}
		}

		return &FAudioWidgetsStyle::Get().GetWidgetStyle<FAudioMaterialButtonStyle>("AudioMaterialButton.Style");
	}

	/** Get the AudioMaterialMeter Style. If MeterStyleOverride is not set, returns default style.*/
	const FAudioMaterialMeterStyle* GetMeterStyle() const
	{
		if (const UObject* Style = MeterStyleOverride.TryLoad())
		{
			if (const USlateWidgetStyleAsset* SlateWidgetStyleAsset = CastChecked<USlateWidgetStyleAsset>(Style))
			{
				return SlateWidgetStyleAsset->GetStyle<FAudioMaterialMeterStyle>();
			}
		}

		return &FAudioWidgetsStyle::Get().GetWidgetStyle<FAudioMaterialMeterStyle>("AudioMaterialMeter.Style");
	}
};
