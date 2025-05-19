// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#include "Runtime/Launch/Resources/Version.h"


#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION >= 6)	//this entire class is only available in 5.6 and up
#include "MetasoundOperatorData.h"
#include "MetasoundFrontendDocument.h"
#endif

#include "unDAW_ConfigurableMixer.generated.h"

USTRUCT()
struct FConfigurableMixerConfiguration  :public FMetaSoundFrontendNodeConfiguration
{
	GENERATED_BODY()



	/* The number of inputs to create. */
	UPROPERTY(EditAnywhere, Category = General, meta = (ClampMin = "2", ClampMax = "1000"))
	uint32 NumInputs;

	UPROPERTY(EditAnywhere, Category = General, meta = (DisplayName = "Equal Power Panning Law"))
	bool bEqualPower;

	FConfigurableMixerConfiguration();

	/* Get the current interface for the class based upon the node extension */
	virtual TInstancedStruct<FMetasoundFrontendClassInterface> OverrideDefaultInterface(const FMetasoundFrontendClass& InNodeClass) const;

	///* Pass data down to the operator. */
	virtual TSharedPtr<const Metasound::IOperatorData> GetOperatorData() const override;

};


//#endif