// Copyright Epic Games, Inc. All Rights Reserved.

#include "BKEditorUtilities.h"

#include "ContentBrowserModule.h"

#include "UObject/UObjectArray.h"
#include "SequenceDataFactory/BKMusicSequenceDataFactory.h"
#include "Serialization/JsonSerializer.h"

#define LOCTEXT_NAMESPACE "BKEditorUtilitiesModule"

void BKEditorUtilitiesModule::StartupModule()
{
	//GlyphsJSON.Get()->TryGetField(TEXT("noteheadBlack")).Get()->AsObject()->TryGetField(TEXT("codepoint")).Get()->AsString();
	SFZAssetTypeActions = MakeShared<FFksfzaSampleBankAssetActions>();
	DAWAssetTypeActions = MakeShared<FDAWSequenceAssetActions>();
	FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(SFZAssetTypeActions.ToSharedRef());
	FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(DAWAssetTypeActions.ToSharedRef());
	//FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(FKMidiAssetTypeActions.ToSharedRef());

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&BKEditorUtilitiesModule::OnExtendMidiAssetSelectionMenu));

	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	const auto& section = PropertyModule.FindOrCreateSection("Actor", "unDAW", INVTEXT("unDAW"));
	section->AddCategory("unDAW");
	section->AddCategory("BK Music");

	PropertyModule.RegisterCustomClassLayout("DAWSequencerData", FOnGetDetailCustomizationInstance::CreateStatic(&FSequenceAssetDetails::MakeInstance));

	const auto& componentSection = PropertyModule.FindOrCreateSection("ActorComponent", "unDAW", INVTEXT("unDAW"));
	componentSection->AddCategory("unDAW");
	componentSection->AddCategory("BK Music");

	//graph thingies
	M2SoundGraphPanelNodeFactory = MakeShareable(new FM2SoundGraphPanelNodeFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(M2SoundGraphPanelNodeFactory);
}

void BKEditorUtilitiesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
	FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(SFZAssetTypeActions.ToSharedRef());
	FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(DAWAssetTypeActions.ToSharedRef());
	//FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(FKMidiAssetTypeActions.ToSharedRef());
}

void BKEditorUtilitiesModule::AddPianoRollToMidiAsset(FMenuBuilder& MenuBuilder,
	const TArray<FAssetData> SelectedAssets)
{
	for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		const FAssetData& Asset = *AssetIt;

		if (!Asset.IsRedirector())
		{
			if (Asset.GetClass()->IsChildOf(UMidiFile::StaticClass()))
			{
				MenuBuilder.BeginSection("BK Midi Utils", FText::FromString("BK Midi Editor"));
				{
					MenuBuilder.AddMenuEntry(
						INVTEXT("Create unDAW Sequence Data From MidiAsset"),
						INVTEXT("Creates creates a new asset and populates it with the info from the midi file!"),
						FSlateIcon(),//FLinterStyle::GetStyleSetName(), "Linter.Toolbar.Icon"),
						FUIAction(FExecuteAction::CreateLambda([Asset]()
							{
								// Do work here.
								OpenSelectedMidiFileInEditorWidget(Asset.GetSoftObjectPath());
							})),
						NAME_None,
						EUserInterfaceActionType::Button);
				}
				MenuBuilder.EndSection();
			}
		}
	}
}

TSharedRef<FExtender> BKEditorUtilitiesModule::OnExtendMidiAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddMenuExtension(
		"CommonAssetActions",
		EExtensionHook::Before,
		nullptr,
		FMenuExtensionDelegate::CreateStatic(&BKEditorUtilitiesModule::AddPianoRollToMidiAsset, SelectedAssets)
	);
	return Extender;
}

void BKEditorUtilitiesModule::OpenSelectedMidiFileInEditorWidget(FSoftObjectPath MidiFileSoftPath)
{
	//OpenSelectedMidiFileInEditorWidget(MidiFileSoftPath);
	UObject* MidiFileObject = MidiFileSoftPath.TryLoad();
	if (MidiFileObject != nullptr)
	{
		UMidiFile* ObjectAsMidi = Cast<UMidiFile>(MidiFileObject);
		if (ObjectAsMidi != nullptr)
		{
			FString NewAssetName = ObjectAsMidi->GetName() + TEXT("_DAWSequence");
			FString PackagePath = TEXT("/Game/DAWSequences/");
			FAssetToolsModule& Module = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
			auto NewAsset = Module.Get().CreateAssetWithDialog(NewAssetName, PackagePath, UDAWSequencerData::StaticClass(), NewObject<UBKMusicSequenceDataFactory>());

			if (NewAsset != nullptr)
			{
				UDAWSequencerData* NewSequenceData = Cast<UDAWSequencerData>(NewAsset);
				NewSequenceData->PopulateFromMidiFile(ObjectAsMidi);
				Module.Get().OpenEditorForAssets(TArray<UObject*>{NewAsset});
			}
			//BKEditorUtilitiesModule::OpenSelectedMidiFileInEditorWidget(ObjectAsMidi);
		}
	};
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(BKEditorUtilitiesModule, BKEditorUtilities)