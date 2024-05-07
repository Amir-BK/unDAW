
#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Widgets/Layout/SScrollBox.h"
#include "EditorUndoClient.h"
#include "SequencerData.h"
#include "GraphEditor.h"


class UConcordModel;
class UConcordModelGraph;
class UConcordModelGraphOutput;
class FConcordSampler;

class FUnDAWSequenceEditorToolkit : public FAssetEditorToolkit
{
public:
    void InitEditor(const TArray<UObject*>& InObjects);
 
    void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
    void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
 
    FName GetToolkitFName() const override { return "NormalDistributionEditor"; }
    FText GetBaseToolkitName() const override { return INVTEXT("Normal Distribution Editor"); }
    FString GetWorldCentricTabPrefix() const override { return "Normal Distribution "; }
    FLinearColor GetWorldCentricTabColorScale() const override { return {}; }


protected:
    UDAWSequencerData* SequenceData;
};
