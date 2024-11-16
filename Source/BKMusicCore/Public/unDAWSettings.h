#pragma once

#include "Engine/DeveloperSettings.h"
#include "M2SoundGraphStatics.h"
#include "AudioMaterialSlate/AudioMaterialSlateTypes.h"
#include "AudioWidgetsEnums.h"


#include "unDAWSettings.generated.h"

UENUM()
enum class EFloatPinWidgetType :  uint8 
{
	Slider,
	Knob,
	NoWidget,
};

USTRUCT()
struct FM2SoundFloatPinConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EFloatPinWidgetType WidgetType = EFloatPinWidgetType::Knob;

	//unit type
	UPROPERTY(EditAnywhere)
	EAudioUnitsValueType UnitType = EAudioUnitsValueType::Linear;


	/**Override the Knob Style used in the Metasound Editor.*/
	UPROPERTY(EditAnywhere, config, Category = "Widget Styling (Experimental)", meta = (AllowedClasses = "/Script/SlateCore.SlateWidgetStyleAsset", DisplayName = "Knob Style"))
	FSoftObjectPath KnobStyleOverride;

	/**Override the Slider Style used in the Metasound Editor.*/
	UPROPERTY(EditAnywhere, config, Category = "Widget Styling (Experimental)", meta = (AllowedClasses = "/Script/SlateCore.SlateWidgetStyleAsset", DisplayName = "Slider Style"))
	FSoftObjectPath SliderStyleOverride;

	//knob style, slider style
	//UPROPERTY()

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EOrientation> SliderOrientation = EOrientation::Orient_Horizontal;

	UPROPERTY(EditAnywhere)
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere)
	float MaxValue = 1.0f;


};

USTRUCT()
struct FCachedM2SoundVertex
{
	GENERATED_BODY()
};

USTRUCT()
struct FCachedVertexPinInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TMap<FName, FFloatRange> PinRanges;

	UPROPERTY(EditAnywhere)
	TMap<FName, FM2SoundFloatPinConfig> FloatPinConfigs;
};

UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "unDAW Settings"))
class BKMUSICCORE_API UUNDAWSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	static UUNDAWSettings* Get() { return CastChecked<UUNDAWSettings>(UUNDAWSettings::StaticClass()->GetDefaultObject()); }

	UUNDAWSettings();



	// Cache for the ranges of the pins on the m2sound graph nodes,
	UPROPERTY(Config, EditAnywhere, Category = "Patch Cache")
	TMap<FName, FCachedVertexPinInfo> Cache;

	/** Maps Pin Category To Pin Color */
	TMap<FName, FLinearColor> CustomPinTypeColors;

	/** Default pin type color */
	UPROPERTY(EditAnywhere, config, Category = PinColors)
	FLinearColor DefaultPinTypeColor;

	/** Audio pin type color */
	UPROPERTY(EditAnywhere, config, Category = PinColors)
	FLinearColor AudioPinTypeColor;

	/** Boolean pin type color */
	UPROPERTY(EditAnywhere, config, Category = PinColors)
	FLinearColor BooleanPinTypeColor;

	/** Floating-point pin type color */
	UPROPERTY(EditAnywhere, config, Category = PinColors)
	FLinearColor FloatPinTypeColor;

	/** Integer pin type color */
	UPROPERTY(EditAnywhere, config, Category = PinColors)
	FLinearColor IntPinTypeColor;

	/** Object pin type color */
	UPROPERTY(EditAnywhere, config, Category = PinColors)
	FLinearColor ObjectPinTypeColor;

	/** String pin type color */
	UPROPERTY(EditAnywhere, config, Category = PinColors)
	FLinearColor StringPinTypeColor;

	/** Time pin type color */
	UPROPERTY(EditAnywhere, config, Category = PinColors)
	FLinearColor TimePinTypeColor;

	/** Trigger pin type color */
	UPROPERTY(EditAnywhere, config, Category = PinColors)
	FLinearColor TriggerPinTypeColor;

	/** WaveTable pin type color */
	UPROPERTY(EditAnywhere, config, Category = PinColors)
	FLinearColor WaveTablePinTypeColor;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};