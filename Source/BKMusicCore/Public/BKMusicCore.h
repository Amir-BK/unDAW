
#pragma once

#include "CoreMinimal.h"
//#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "MetasoundDocumentInterface.h"
#include "WindowsTargetSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
//#include "BKMusicCore.generated.h"



DECLARE_LOG_CATEGORY_EXTERN(LogBKMusicCore, Log, All);

class BKMusicCoreModule;



namespace UnDAW
{
	class IMetasoundAssetListener
	{
		friend class BKMusicCoreModule;

	protected:
		virtual void MetasoundDocumentUpdated() = 0;
		virtual void SetMetasoundAsset(const FMetasoundFrontendDocument* InMetasound)
		{
			this->Metasound = InMetasound;
		}

	public:

		const FMetasoundFrontendDocument* GetMetasound() const
		{
			return Metasound;
		}

	private:
		const FMetasoundFrontendDocument* Metasound;

		// can be used by editor modules that want to receive updates for any metasound asset change.
		bool bGlobalListener = false;
	};

}



class BKMusicCoreModule : public IModuleInterface
{
public:
	FString PluginContentDir;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void RegisterMetasoundAssetListener(UnDAW::IMetasoundAssetListener* Listener);
	static void UnregisterMetasoundAssetListener(UnDAW::IMetasoundAssetListener* Listener);

	void OnAssetsChanged(TConstArrayView<FAssetData> InUpdatedAssets);

	void OnMetasoundAssetUpdated(UObject* AssetObject);

private:
	TArray<UnDAW::IMetasoundAssetListener*> MetasoundAssetListeners;

	bool bMetasoundRegistryNeedsRebuilding = true;
};
