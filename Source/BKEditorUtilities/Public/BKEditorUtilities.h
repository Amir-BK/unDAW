// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"
#include "FFKSFZASampleBankAssetActions.h"
#include "UnDawSequenceAssetActions.h"
#include "SequenceDataFactory/BKMusicSequenceDataFactory.h"


class BKEditorUtilitiesModule final : public IModuleInterface
{
public:
	
	FString PluginContentDir;






	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void AddPianoRollToMidiAsset(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets);

	static TSharedRef<FExtender> OnExtendMidiAssetSelectionMenu (const TArray<FAssetData>& SelectedAssets);
	static void OpenSelectedMidiFileInEditorWidget(FSoftObjectPath MidiFileSoftPath);

	static void OpenSelectedMidiFileInEditorWidget(UMidiFile* MidiFilePointer);

private:
	TSharedPtr< FFksfzaSampleBankAssetActions> SFZAssetTypeActions;
	TSharedPtr< FFKMidiEditorAssetActions> FKMidiAssetTypeActions;
	TSharedPtr< FDAWSequenceAssetActions> DAWAssetTypeActions;
};
