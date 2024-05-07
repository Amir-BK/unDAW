// Copyright 2022 Jan Klimaschewski. All Rights Reserved.

#include "UnDAWSequenceEditorToolkit.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"

void FUnDAWSequenceEditorToolkit::InitEditor(const TArray<UObject*>& InObjects)
{
    SequenceData = Cast<UDAWSequencerData>(InObjects[0]);

    const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("NormalDistributionEditorLayout")
        ->AddArea
        (
            FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
            ->Split
            (
                FTabManager::NewSplitter()
                ->SetSizeCoefficient(0.6f)
                ->SetOrientation(Orient_Horizontal)
                ->Split
                (
                    FTabManager::NewStack()
                    ->SetSizeCoefficient(0.8f)
                    ->AddTab("NormalDistributionPDFTab", ETabState::OpenedTab)
                    ->AddTab("PianoRollTab", ETabState::ClosedTab)
                )
                ->Split
                (
                    FTabManager::NewStack()
                    ->SetSizeCoefficient(0.2f)
                    ->AddTab("NormalDistributionDetailsTab", ETabState::OpenedTab)
                )
            )
            ->Split
            (
                FTabManager::NewStack()
                ->SetSizeCoefficient(0.4f)
                ->AddTab("OutputLog", ETabState::OpenedTab)
            )
        );
    FAssetEditorToolkit::InitAssetEditor(EToolkitMode::Standalone, {}, "NormalDistributionEditor", Layout, true, true, InObjects);
    

}

void FUnDAWSequenceEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
 
    WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(INVTEXT("Normal Distribution Editor"));
 
    InTabManager->RegisterTabSpawner("NormalDistributionPDFTab", FOnSpawnTab::CreateLambda([=](const FSpawnTabArgs&)
    {
        return SNew(SDockTab)
        [
            SNew(STextBlock)
                .Text(INVTEXT("TEST!"))

        ];
    }))
    .SetDisplayName(INVTEXT("PDF"))
    .SetGroup(WorkspaceMenuCategory.ToSharedRef());

    InTabManager->RegisterTabSpawner("PianoRollTab", FOnSpawnTab::CreateLambda([&](const FSpawnTabArgs&)
        {
            auto DockTab = SNew(SDockTab)
                [
                    SAssignNew(PianoRollGraph, SPianoRollGraph)
                      .SessionData(SequenceData->GetSelfSharedPtr())
                        .Clipping(EWidgetClipping::ClipToBounds)
                        //.gridBrush(GridBrush)
                        .gridColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("8A8A8A00"))))
                        .accidentalGridColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("00000082"))))
                        .cNoteColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FF33E220"))))
                ];

            if (SequenceData->HarmonixMidiFile) PianoRollGraph->Init();

            SequenceData->OnMidiDataChanged.AddLambda([&]()
                {
				PianoRollGraph->Init();
			});

            return DockTab;
        }))
        .SetDisplayName(INVTEXT("PianoRoll"))
            .SetGroup(WorkspaceMenuCategory.ToSharedRef());

        //if (SequenceData->HarmonixMidiFile) PianoRollGraph->Init();
 
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    DetailsView->SetObjects(TArray<UObject*>{ SequenceData });
    InTabManager->RegisterTabSpawner("NormalDistributionDetailsTab", FOnSpawnTab::CreateLambda([=](const FSpawnTabArgs&)
    {
        return SNew(SDockTab)
        [
            DetailsView
        ];
    }))
    .SetDisplayName(INVTEXT("Details"))
    .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}
 
void FUnDAWSequenceEditorToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
    InTabManager->UnregisterTabSpawner("NormalDistributionPDFTab");
    InTabManager->UnregisterTabSpawner("NormalDistributionDetailsTab");
}

inline FUnDAWSequenceEditorToolkit::~FUnDAWSequenceEditorToolkit()
{
    SequenceData->OnMidiDataChanged.RemoveAll(this);
    PianoRollGraph.Reset();
}

void FSequenceAssetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Outers;
	DetailBuilder.GetObjectsBeingCustomized(Outers);
	if (Outers.Num() == 0) return;
	SequenceData = Cast<UDAWSequencerData>(Outers[0].Get());

	DetailBuilder.EditCategory("Un DAW")
	    .AddCustomRow(FText::FromString("Tracks"))
		.WholeRowContent()
        [
			SNew(SButton)
			.Text(FText::FromString(SequenceData->HarmonixMidiFile->GetName()))
              
		];
}
