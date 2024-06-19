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

UCLASS(config = EditorPerProjectUserSettings, meta=(DisplayName = "unDAW Settings"))
class BKMUSICCORE_API UUNDAWSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	static UUNDAWSettings* Get() { return CastChecked<UUNDAWSettings>(UUNDAWSettings::StaticClass()->GetDefaultObject()); }

	
	UPROPERTY(Config, EditAnywhere, Category = "Patch Cache")
	TMap<FName, FCachedVertexPinInfo> Cache;

};