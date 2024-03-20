// Copyright Epic Games, Inc. All Rights Reserved.

#include "unDAWMetaSounds.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
#include "MetasoundUObjectRegistry.h"
#include "Misc/AssetRegistryInterface.h"
#include "MetasoundSource.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"
#include "MetasoundDataTypeRegistrationMacro.h"


#define LOCTEXT_NAMESPACE "unDAWMetaSoundsModule"



void unDAWMetaSoundsModule::StartupModule()
{
	
	//using namespace Metasound::Engine;

	////GlyphsJSON.Get()->TryGetField(TEXT("noteheadBlack")).Get()->AsObject()->TryGetField(TEXT("codepoint")).Get()->AsString();
	//Metasound::RegisterDataTypeWithFrontend<Metasound::F_FK_SFZ_Instrument_Asset, Metasound::ELiteralType::UObjectProxy, UFKSFZAsset>();
	//Metasound::RegisterDataTypeWithFrontend<Metasound::F_FK_SFZ_Region_Data, Metasound::ELiteralType::UObjectProxy, UFK_Region_Runtime_Performance_Data>();
	////Metasound::RegisterDataTypeWithFrontend<Metasound::FFKSFKInstrument, Metasound::ELiteralType::UObjectProxy, UFKSFZSample>();
	////Metasound::RegisterNodeWithFrontend<Metasound::FKSFZSamplePlayerNode>();
	//
	//FMetasoundFrontendRegistryContainer::Get()->RegisterPendingNodes();

	unDAW::Metasounds::FunDAWInstrumentRendererInterface::RegisterInterface();
	
}

void unDAWMetaSoundsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(unDAWMetaSoundsModule, unDAWMetaSounds)