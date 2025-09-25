// Copyright Epic Games, Inc. All Rights Reserved.

#include "unDAWMetaSounds.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
#include "MetasoundUObjectRegistry.h"
#include "Misc/AssetRegistryInterface.h"
#include "MetasoundSource.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "Misc/EngineVersionComparison.h"
#include "MetasoundDataTypeRegistrationMacro.h"

#define LOCTEXT_NAMESPACE "unDAWMetaSoundsModule"

void unDAWMetaSoundsModule::StartupModule()
{
#if !(UE_VERSION_OLDER_THAN(5, 7, 0))	
	METASOUND_REGISTER_ITEMS_IN_MODULE
#endif
}

void unDAWMetaSoundsModule::ShutdownModule()
{
#if !(UE_VERSION_OLDER_THAN(5, 7, 0))	
	METASOUND_UNREGISTER_ITEMS_IN_MODULE
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(unDAWMetaSoundsModule, unDAWMetaSounds)