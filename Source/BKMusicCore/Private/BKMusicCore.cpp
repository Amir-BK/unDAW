// Copyright Epic Games, Inc. All Rights Reserved.

#include "BKMusicCore.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"
//#include "MetasoundUObjectRegistry.h"

#include "MetasoundDataTypeRegistrationMacro.h"


#define LOCTEXT_NAMESPACE "BKMusicCoreModule"

void BKMusicCoreModule::StartupModule()
{
#ifdef WITH_CHUNREAL_PLUGIN
	UE_LOG(LogTemp, Warning, TEXT("WE SEE CHUNREAL!"));
#endif 

	unDAW::Metasounds::FunDAWInstrumentRendererInterface::RegisterInterface();
	unDAW::Metasounds::FunDAWCustomInsertInterface::RegisterInterface();
	unDAW::Metasounds::FunDAWMasterGraphInterface::RegisterInterface();
	unDAW::Metasounds::FunDAWMidiInsertInterface::RegisterInterface();
	unDAW::Metasounds::FunDAWMusicalActionInterface::RegisterInterface();
	unDAW::Metasounds::FunDAWAudibleActionInterface::RegisterInterface();



}

void BKMusicCoreModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(BKMusicCoreModule, BKMusicCore)