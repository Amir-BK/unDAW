
#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Widgets/Layout/SScrollBox.h"
#include "EditorUndoClient.h"
#include "SequencerData.h"
#include "SPianoRollGraph.h"

#include "GlyphButton.h"

#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "GraphEditor.h"


class FSequenceAssetDetails : public IDetailCustomization
{
public:
    // This function will be called when the properties are being customized
    void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

    static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShareable(new FSequenceAssetDetails()); }
private:
    UDAWSequencerData* SequenceData = nullptr;

    TSharedPtr<SVerticalBox> MidiInputTracks;

    void UpdateMidiInputTracks();

    // This function will create a new instance of this class as a shared pointer
};


class FUnDAWSequenceEditorToolkit : public FAssetEditorToolkit, public IBK_MusicSceneManagerInterface
{
public:
    void InitEditor(const TArray<UObject*>& InObjects);
 
    void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
    void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

    TSharedRef<SButton> GetConfiguredTransportButton(EBKTransportCommands InCommand);
 
    FName GetToolkitFName() const override { return "unDAWSequenceEditor"; }
    FText GetBaseToolkitName() const override { return INVTEXT("unDAW Sequence Editor"); }
    FString GetWorldCentricTabPrefix() const override { return "unDAW "; }
    FLinearColor GetWorldCentricTabColorScale() const override { return {}; }


    void ExtendToolbar();

    ~FUnDAWSequenceEditorToolkit();

protected:
    UDAWSequencerData* SequenceData;

    TSharedPtr<SPianoRollGraph> PianoRollGraph;

    TSharedPtr<STextBlock> CurrentPlayStateTextBox;
    TSharedPtr<SHorizontalBox> TransportControls;

    void PreviewAudio();
    void PlayAudioComponent();
    void StopAudioComponent();

    UAudioComponent* AudioComponent = nullptr;
};
