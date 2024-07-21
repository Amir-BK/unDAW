#include "unDAWK2_Nodes.h"

#define LOCTEXT_NAMESPACE "FunDAWK2_NodesModule"

void FunDAWK2_NodesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FunDAWK2_NodesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FunDAWK2_NodesModule, unDAWK2_Nodes)