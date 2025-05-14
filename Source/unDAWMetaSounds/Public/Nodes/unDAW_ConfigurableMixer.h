// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MetasoundOperatorData.h"
#include "MetasoundFrontendDocument.h"

#include "unDAW_ConfigurableMixer.generated.h"

USTRUCT()
struct FConfigurableMixerConfiguration : public FMetaSoundFrontendNodeConfiguration
{
	GENERATED_BODY()

	FConfigurableMixerConfiguration();

	/* The number of inputs to create. */
	UPROPERTY(EditAnywhere, Category = General, meta = (ClampMin = "2", ClampMax = "1000"))
	uint32 NumInputs;

	/* Get the current interface for the class based upon the node extension */
	virtual TInstancedStruct<FMetasoundFrontendClassInterface> OverrideDefaultInterface(const FMetasoundFrontendClass& InNodeClass) const;

	///* Pass data down to the operator. */
	virtual TSharedPtr<const Metasound::IOperatorData> GetOperatorData() const override;
};
