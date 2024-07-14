// Copyright Epic Games, Inc. All Rights Reserved.

#include "unPatchwork.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
//#include "MetasoundUObjectRegistry.h"

#define LOCTEXT_NAMESPACE "unPatchworkModule"

void unPatchworkModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("Hello unPatchwork"))
}

void unPatchworkModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(unPatchworkModule, unPatchwork)