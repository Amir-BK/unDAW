// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"
#include "FFKSFZASampleBankAssetActions.h"
#include "M2SoundEdGraphSchema.h"
#include "SequenceDataFactory/BKMusicSequenceDataFactory.h"

class BKEditorUtilitiesModule final : public IModuleInterface
{
public:

	FString PluginContentDir;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void AddPianoRollToMidiAsset(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets);

	static TSharedRef<FExtender> OnExtendMidiAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
	static void OpenSelectedMidiFileInEditorWidget(FSoftObjectPath MidiFileSoftPath);

private:
	
	//TSharedPtr< FFKMidiEditorAssetActions> FKMidiAssetTypeActions;
	TSharedPtr< FDAWSequenceAssetActions> DAWAssetTypeActions;
	TSharedPtr< FM2SoundGraphPanelNodeFactory> M2SoundGraphPanelNodeFactory;
};
