// Copyright Epic Games, Inc. All Rights Reserved.

#include "BKMusicCore.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
//#include "MetasoundUObjectRegistry.h"

#include "MetasoundDataTypeRegistrationMacro.h"


#define LOCTEXT_NAMESPACE "BKMusicCoreModule"

void BKMusicCoreModule::StartupModule()
{

}

void BKMusicCoreModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(BKMusicCoreModule, BKMusicCore)