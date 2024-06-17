#pragma once

#include "Engine/DeveloperSettings.h"

#include "unDAWSettings.generated.h"


UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "unDAW_Test"))
class BKMUSICCORE_API UunDAWTestSettings : public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()

public:

	//UunDAWTestSettings();

	static UunDAWTestSettings* Get() { return CastChecked<UunDAWTestSettings>(UunDAWTestSettings::StaticClass()->GetDefaultObject()); }

	UPROPERTY(Config, EditAnywhere, Category = "Testing")
	FString MyTestString;


#if WITH_EDITORONLY_DATA
	virtual FName GetCategoryName() const override { return FName("unDAW"); }
	virtual FText GetSectionText() const override { return INVTEXT("Test Settings"); }
#endif

};