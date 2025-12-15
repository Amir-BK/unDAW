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
#if WITH_EDITOR
	AssetRegistryModule.Get().OnAssetsUpdatedOnDisk().AddRaw(this, &BKMusicCoreModule::OnAssetsChanged);
#endif // WITH_EDITOR



}

void BKMusicCoreModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	//FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	//AssetRegistryModule.Get().OnAssetsUpdatedOnDisk().RemoveAll(this);
}

#if WITH_EDITOR

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
	//BKMusicCoreModule& Module = FModuleManager::LoadModuleChecked<BKMusicCoreModule>("BKMusicCore");
	//remove the listener from the array
	//Module.MetasoundAssetListeners.Remove(Listener);

}



void BKMusicCoreModule::OnAssetsChanged(TConstArrayView<FAssetData> InUpdatedAssets)
{

	for (const FAssetData& AssetData : InUpdatedAssets)
	{
		UClass* AssetClass = AssetData.GetClass(EResolveClass::Yes);

		//print asset name and class
		UE_LOG(LogBKMusicCore, Warning, TEXT("Asset Changed: %s, Class: %s"), *AssetData.AssetName.ToString(), *AssetClass->GetName());

		using namespace UnDAW;
		if (AssetClass == UMetaSoundPatch::StaticClass() || AssetClass == UMetaSoundSource::StaticClass())
		{
			//get the asset object
			OnMetasoundAssetUpdated(AssetData.GetAsset());
		}

		
	}




}

inline void BKMusicCoreModule::OnMetasoundAssetUpdated(UObject* AssetObject)
{
	for (auto Listener : MetasoundAssetListeners)
	{
		if (IMetaSoundDocumentInterface* CastedAsset = Cast<IMetaSoundDocumentInterface>(AssetObject))
		{
			//does document match?!?! I guess I forgot about that
			if (Listener->bGlobalListener || Listener->GetMetasound() == &CastedAsset->GetConstDocument())
			{
				Listener->SetMetasoundAsset(&CastedAsset->GetConstDocument());
				Listener->MetasoundDocumentUpdated();
			}

		}

	}
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(BKMusicCoreModule, BKMusicCore)