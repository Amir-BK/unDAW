#pragma once

#include "Engine/DeveloperSettings.h"
#include "M2SoundGraphStatics.h"

#include "unDAWSettings.generated.h"

USTRUCT()
struct FCachedVertexPinInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TMap<FName, FFloatRange> PinRanges;

};

UCLASS(config = Game, meta=(DisplayName = "unDAW Settings"))
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

};