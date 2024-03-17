// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "AssetTypeActions_Base.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "MIDIEditorBase.h"
#include "Blueprint/WidgetTree.h"
#include "HarmonixMidi/MidiFile.h"
//#include "FKMidiEditorAssetActions.generated.h"

/**
 * 
 */

class BK_EDITORUTILITIES_API FFKMidiEditorAssetActions final : public FAssetTypeActions_Base
{


public:
	bool HasActions(const TArray<UObject*>& InObjects) const
	{
		return true;
	}

	void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override
	{

		//add harmonix actions 
		auto MidiAssets = GetTypedWeakObjectPtrs<UMidiFile>(InObjects);

	
			FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
			TSharedPtr<IAssetTypeActions> EngineActions = AssetToolsModule.Get().GetAssetTypeActionsForClass(GetSupportedClass()).Pin();
			if (EngineActions.IsValid())
			{
				EngineActions->GetActions(InObjects, MenuBuilder);
			}
		/*
		 * We are actually creating a delegate. The delegate will get passed on to FUIAction as a constructor argument.
		 * Currently, we are assuming FUIAction will call the Broadcast,or Execute() functions when appropriate(...On Click).
		 * When we create this delegate, we immediately bind a function to it, with CreateSP, the function is the function to call on click.
		 * Our Reimport CallBackFunction. It will pass the selected MidiAssets, as weak pointers, on to our callback function.
		 *
		 *
		 */
		auto TDelegateExecuteAction = FExecuteAction::CreateSP(this, &FFKMidiEditorAssetActions::OpenMidiInFKPianoRoll, MidiAssets);

		auto UIAction = FUIAction(TDelegateExecuteAction);


		const FText ButtonLabel = FText::FromString("Edit MIDI Asset in Piano Roll View");
		const FText ButtonToolTip = FText::FromString("Allows modifying an existing midi asset or creating a new one within UE!");



		MenuBuilder.AddMenuEntry(ButtonLabel, ButtonToolTip, FSlateIcon(), UIAction);
		
	};



	void OpenMidiInFKPianoRoll(TArray<TWeakObjectPtr<UMidiFile>> MidiFilePointer)
	{
		for (auto MidiAssetWeakPtr : MidiFilePointer)
		{
			/*
			UMidiAsset* MidiAsset = MidiAssetWeakPtr.Get();
	
	
			MidiAsset->ShiftOctaveUp();
	
	
			//Mark the package dirty so that it can be saved.
			bool bMarkedForSave = MidiAsset->MarkPackageDirty();
			*/
			const FSoftObjectPath widgetAssetPath("/Script/Blutility.EditorUtilityWidgetBlueprint/Script/Blutility.EditorUtilityWidgetBlueprint'/FKAdaptiveMusic/EditorWidget/PianoRoll/EUW_PianoRoll.EUW_PianoRoll'");

			UObject* widgetAssetLoaded = widgetAssetPath.TryLoad();
			if (widgetAssetLoaded == nullptr) {
				UE_LOG(LogTemp, Warning, TEXT("Missing Expected widget class at : /LevelValidationTools/EUW_LevelValidator.EUW_LevelValidator"));
				return;
			}

			UEditorUtilityWidgetBlueprint* widget = Cast<UEditorUtilityWidgetBlueprint>(widgetAssetLoaded);

			if (widget == nullptr) {
				UE_LOG(LogTemp, Warning, TEXT("Couldnt cast /LevelValidationTools/EUW_LevelValidator.EUW_LevelValidator to UEditorUtilityWidgetBlueprint"));
				return;
			}


			UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
			auto spawnedWidget = EditorUtilitySubsystem->SpawnAndRegisterTab(widget);
	
			auto EngPanel = spawnedWidget->WidgetTree.Get()->FindWidget(FName(TEXT("MIDIEditorBase")));
			UMIDIEditorBase* widgetAsEngraving = Cast<UMIDIEditorBase>(EngPanel);
			if (widgetAsEngraving == nullptr) {
				//spawnedWidget->GetRootWidget()->Slot.Get().
				UE_LOG(LogTemp, Warning, TEXT("Couldnt cast to base CPP class"));
				return;
			}
	

			widgetAsEngraving->HarmonixMidiFile = MidiAssetWeakPtr;
			widgetAsEngraving->InitFromDataHarmonix();
		
		};
	}

	UClass* GetSupportedClass() const override
	{
		return UMidiFile::StaticClass();
	}

	FColor GetTypeColor() const override
	{
		return FColor::Yellow;
	}

	uint32 GetCategories() override
	{
		return EAssetTypeCategories::Sounds;
	}
	
	FText GetName() const
	{
		return INVTEXT("Midi File");
	}
};
