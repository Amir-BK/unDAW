// Copyright Epic Games, Inc. All Rights Reserved.

#include "BKMusicCore.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
#include "MetasoundSource.h"
#include "Metasound.h"
#include "MetasoundAssetSubsystem.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"
//#include "MetasoundUObjectRegistry.h"

#include "MetasoundDataTypeRegistrationMacro.h"


#define LOCTEXT_NAMESPACE "BKMusicCoreModule"

DEFINE_LOG_CATEGORY(LogBKMusicCore);

void BKMusicCoreModule::StartupModule()
{
#ifdef WITH_CHUNREAL_PLUGIN
	UE_LOG(LogBKMusicCore, Warning, TEXT("WE SEE CHUNREAL!"));
#endif 

	unDAW::Metasounds::FunDAWInstrumentRendererInterface::RegisterInterface();
	unDAW::Metasounds::FunDAWCustomInsertInterface::RegisterInterface();
	unDAW::Metasounds::FunDAWMasterGraphInterface::RegisterInterface();
	unDAW::Metasounds::FunDAWMidiInsertInterface::RegisterInterface();
	unDAW::Metasounds::FunDAWMusicalActionInterface::RegisterInterface();
	unDAW::Metasounds::FunDAWAudibleActionInterface::RegisterInterface();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	//AssetRegistryModule.Get().OnAssetUpdated().AddRaw(this, &BKMusicCoreModule::OnAssetChanged);
	AssetRegistryModule.Get().OnAssetsUpdatedOnDisk().AddRaw(this, &BKMusicCoreModule::OnAssetsChanged);




}

void BKMusicCoreModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void BKMusicCoreModule::RegisterMetasoundAssetListener(UnDAW::IMetasoundAssetListener* Listener)
{
	//get static access to the module
	BKMusicCoreModule& Module = FModuleManager::LoadModuleChecked<BKMusicCoreModule>("BKMusicCore");

	//add the listener to the array
	Module.MetasoundAssetListeners.AddUnique(Listener);
}

void BKMusicCoreModule::UnregisterMetasoundAssetListener(UnDAW::IMetasoundAssetListener* Listener)
{
	//get static access to the module
	BKMusicCoreModule& Module = FModuleManager::LoadModuleChecked<BKMusicCoreModule>("BKMusicCore");
	//remove the listener from the array
	Module.MetasoundAssetListeners.Remove(Listener);
}



void BKMusicCoreModule::OnAssetsChanged(TConstArrayView<FAssetData> InUpdatedAssets)
{

	for (const FAssetData& AssetData : InUpdatedAssets)
	{
		UClass* AssetClass = AssetData.GetClass(EResolveClass::Yes);

		//print asset name and class
		UE_LOG(LogBKMusicCore, Warning, TEXT("Asset Changed: %s, Class: %s"), *AssetData.AssetName.ToString(), *AssetClass->GetName());

		using namespace UnDAW;
		if (AssetClass == UMetaSoundPatch::StaticClass())
		{
			UE_LOG(LogBKMusicCore, Warning, TEXT("Asset is a patch"));


			IMetasoundAssetListener** Listener = MetasoundAssetListeners.FindByPredicate([&](IMetasoundAssetListener* Listener)
				{
					return Listener->GetMetasound() == &Cast<UMetaSoundPatch>(AssetData.GetAsset())->GetConstDocument();
				});

			if (Listener)
			{
				(*Listener)->MetasoundDocumentUpdated();
			}

		}
		else if (AssetClass == UMetaSoundSource::StaticClass())
		{

			UE_LOG(LogBKMusicCore, Warning, TEXT("Asset is a source"));

			IMetasoundAssetListener** Listener = MetasoundAssetListeners.FindByPredicate([&](IMetasoundAssetListener* Listener)
				{
					return Listener->GetMetasound() == &Cast<UMetaSoundSource>(AssetData.GetAsset())->GetConstDocument();
				});

			if (Listener)
			{
				(*Listener)->MetasoundDocumentUpdated();
			}


		}
	}




}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(BKMusicCoreModule, BKMusicCore)