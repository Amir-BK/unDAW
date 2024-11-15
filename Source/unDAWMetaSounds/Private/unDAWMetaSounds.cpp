// Copyright Epic Games, Inc. All Rights Reserved.

#include "unDAWMetaSounds.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
#include "MetasoundUObjectRegistry.h"
#include "Misc/AssetRegistryInterface.h"
#include "MetasoundSource.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundDataTypeRegistrationMacro.h"

#define LOCTEXT_NAMESPACE "unDAWMetaSoundsModule"

void unDAWMetaSoundsModule::StartupModule()
{

}

void unDAWMetaSoundsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(unDAWMetaSoundsModule, unDAWMetaSounds)