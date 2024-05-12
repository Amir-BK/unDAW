// Copyright 2022 Jan Klimaschewski. All Rights Reserved.

#include "UnDAWSequenceEditorToolkit.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Modules/ModuleInterface.h"
#include "MetasoundEditorModule.h"
#include "GlyphButton.h"
#include "UnDAWPreviewHelperSubsystem.h"
#include "Widgets/Colors/SColorPicker.h"
#include "SMidiTrackControlsWidget.h"
#include "SEnumCombo.h"


#include "Widgets/Layout/SScaleBox.h"


void FUnDAWSequenceEditorToolkit::InitEditor(const TArray<UObject*>& InObjects)
{
    SequenceData = Cast<UDAWSequencerData>(InObjects[0]);

    ExtendToolbar();

    const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("DAWSequenceEditorLayout")
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
                    ->AddTab("DAWSequenceMixerTab", ETabState::OpenedTab)
                    ->AddTab("PianoRollTab", ETabState::OpenedTab)
                )
                ->Split
                (
                    FTabManager::NewStack()
                    ->SetSizeCoefficient(0.2f)
                    ->AddTab("DAWSequenceDetailsTab", ETabState::OpenedTab)
                )
            )
            ->Split
            (
                FTabManager::NewStack()
                ->SetSizeCoefficient(0.4f)
                ->AddTab("OutputLog", ETabState::OpenedTab)
            )
        );
    FAssetEditorToolkit::InitAssetEditor(EToolkitMode::Standalone, {}, "DAWSequenceEditor", Layout, true, true, InObjects);
    

}

void FUnDAWSequenceEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
 
    WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(INVTEXT("unDAW Sequence Editor"));
 
    InTabManager->RegisterTabSpawner("DAWSequenceMixerTab", FOnSpawnTab::CreateLambda([&](const FSpawnTabArgs&)
    {
        return SNew(SDockTab)
        [
            SNew(STextBlock)
                .Text(INVTEXT("TEST!"))

        ];
    }))
    .SetDisplayName(INVTEXT("Mixer Controls"))
    .SetGroup(WorkspaceMenuCategory.ToSharedRef());

    InTabManager->RegisterTabSpawner("PianoRollTab", FOnSpawnTab::CreateLambda([&](const FSpawnTabArgs&)
        {
            auto DockTab = SNew(SDockTab)
                [
                    SAssignNew(PianoRollGraph, SPianoRollGraph)
                      .SessionData(SequenceData->GetSelfSharedPtr())
                        .Clipping(EWidgetClipping::ClipToBounds)
                        .gridColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("8A8A8A00"))))
                        .accidentalGridColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("00000082"))))
                        .cNoteColor(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FF33E220"))))
                        .OnSeekEvent(OnSeekEvent)
                ];

            //PianoRollGraph->OnSeekEvent.BindLambda([this](float Seek) { OnSeekEvent.ExecuteIfBound(Seek); });

            if (SequenceData->HarmonixMidiFile) {
                PianoRollGraph->Init();
                if (!SequenceData->EditorPreviewPerformer) {
                    SetupPreviewPerformer();
                }
                else {
                    Performer = SequenceData->EditorPreviewPerformer;
                    Performer->OnDeleted.AddLambda([this]() { Performer = nullptr; });
                    }
                }

            SequenceData->OnMidiDataChanged.AddLambda([&]()
                {
				PianoRollGraph->Init();
                SetupPreviewPerformer();
			    });

            return DockTab;
        }))
        .SetDisplayName(INVTEXT("Piano Roll"))
            .SetGroup(WorkspaceMenuCategory.ToSharedRef());

 
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    DetailsView->SetObjects(TArray<UObject*>{ SequenceData });
    InTabManager->RegisterTabSpawner("DAWSequenceDetailsTab", FOnSpawnTab::CreateLambda([=](const FSpawnTabArgs&)
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
    PianoRollGraph->OnSeekEvent.Unbind();
    PianoRollGraph->SessionData.Reset();
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
    InTabManager->UnregisterTabSpawner("DAWSequenceMixerTab");
    InTabManager->UnregisterTabSpawner("DAWSequenceDetailsTab");
    InTabManager->UnregisterTabSpawner("PianoRollTab");
}

TSharedRef<SButton> FUnDAWSequenceEditorToolkit::GetConfiguredTransportButton(EBKTransportCommands InCommand)
{

    auto NewButton = UTransportGlyphButton::CreateTransportButton(InCommand);
    NewButton->SetOnClicked(FOnClicked::CreateLambda([&, InCommand]() { SendTransportCommand(InCommand); return FReply::Handled(); }));

    switch(InCommand)
	{
		case Play:
            NewButton->SetEnabled(TAttribute<bool>::Create([this]() { return Performer != nullptr && (Performer->PlayState == ReadyToPlay || Performer->PlayState == Paused)  ; }));
            NewButton->SetVisibility(TAttribute<EVisibility>::Create([this]() { return Performer != nullptr && ((Performer->PlayState == ReadyToPlay)
                || (Performer->PlayState == Paused)) ? EVisibility::Visible : EVisibility::Collapsed; }));
			break;
		case Stop:

			break;

        case Pause:
            NewButton->SetVisibility(TAttribute<EVisibility>::Create([this]() { return Performer != nullptr && (Performer->PlayState == Playing)
                 ? EVisibility::Visible : EVisibility::Collapsed; }));
			break;
		default:
			break;
	}

    return NewButton;

}

void FUnDAWSequenceEditorToolkit::ExtendToolbar()
{
    TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
    CurrentPlayStateTextBox = SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(UEnum::GetValueAsString(GetCurrentPlaybackState())); })
        .Justification(ETextJustify::Center);

    ToolbarExtender->AddToolBarExtension("Asset",
        EExtensionHook::After,
        GetToolkitCommands(),
        FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
            {
                
                auto StopButton = GetConfiguredTransportButton(Stop);
                auto PlayButton = GetConfiguredTransportButton(Play);
                auto PauseButton = GetConfiguredTransportButton(Pause);

                ToolbarBuilder.BeginSection("Transport");
  
                ToolbarBuilder.AddWidget(SNew(SBox).VAlign(VAlign_Center)
                    [
                        SNew(SHorizontalBox)

                            + SHorizontalBox::Slot()
                            [
                                SNew(SButton)
                                    .Text(INVTEXT("ReInit"))
                                    .OnClicked_Lambda([this]() { SetupPreviewPerformer(); return FReply::Handled(); })
                                    .IsEnabled_Lambda([this]() { return Performer == nullptr; })
                            ]

                            + SHorizontalBox::Slot()
                            [
                                PlayButton
                            ] 
                            + SHorizontalBox::Slot()
                            [
                                PauseButton
                            ]                           
                            + SHorizontalBox::Slot()
                            [
                                StopButton
                            ]
     
                ]);

                ToolbarBuilder.EndSection();

                 ToolbarBuilder.BeginSection("Performer Status");
                 ToolbarBuilder.AddWidget(CurrentPlayStateTextBox.ToSharedRef());
                 ToolbarBuilder.EndSection();

                 ToolbarBuilder.BeginSection("Graph Input");
                 ToolbarBuilder.AddWidget(SNew(SEnumComboBox, StaticEnum<EPianoRollEditorMouseMode>())
                     .CurrentValue_Lambda([this]() -> int32
                 {
                     if (!PianoRollGraph) return (int32)EPianoRollEditorMouseMode::empty;
                     return (int32)PianoRollGraph->InputMode;
                 })
                     .OnEnumSelectionChanged_Lambda([this](int32 NewSelection, ESelectInfo::Type InSelectionInf) { if(PianoRollGraph) PianoRollGraph->SetInputMode(EPianoRollEditorMouseMode(NewSelection)); })

                 );
                 ToolbarBuilder.EndSection();
            }));
    AddToolbarExtender(ToolbarExtender);
}

inline FUnDAWSequenceEditorToolkit::~FUnDAWSequenceEditorToolkit()
{
    SequenceData->OnMidiDataChanged.RemoveAll(this);
    PianoRollGraph->SessionData.Reset();
    PianoRollGraph.Reset();
    SequenceData = nullptr;
}

void FUnDAWSequenceEditorToolkit::SetupPreviewPerformer()
{
    PianoRollGraph->OnSeekEvent.Unbind();
    auto PreviewHelper = GEditor->GetEditorSubsystem<UUnDAWPreviewHelperSubsystem>();
    PreviewHelper->CreateAndPrimePreviewBuilderForDawSequence(SequenceData);

    Performer = SequenceData->EditorPreviewPerformer;
    Performer->OnDeleted.AddLambda([this]() { Performer = nullptr; });
    PianoRollGraph->OnSeekEvent.BindUObject(Performer, &UDAWSequencerPerformer::SendSeekCommand);
    //OnSeekEvent.BindUObject(Performer, &UDAWSequencerPerformer::SendSeekCommand);
    //OnSeekEvent.BindLambda([this](float Seek) { 
    //    UE_LOG(LogTemp, Warning, TEXT("Seeking to %f"), Seek);
    //    
    //    Performer->SendSeekCommand(Seek); });
 
}



void FSequenceAssetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Outers;
	DetailBuilder.GetObjectsBeingCustomized(Outers);
	if (Outers.Num() == 0) return;
	SequenceData = Cast<UDAWSequencerData>(Outers[0].Get());

	DetailBuilder.EditCategory("Tracks")
	    .AddCustomRow(FText::FromString("Tracks"))
		.WholeRowContent()
        [
			SAssignNew(MidiInputTracks, SVerticalBox)      
		];

    UpdateMidiInputTracks();
    SequenceData->OnMidiDataChanged.AddLambda([&]()
        {
			UpdateMidiInputTracks();
		});
}

void FSequenceAssetDetails::OnFusionPatchChangedInTrack(int TrackID, UFusionPatch* NewPatch)
{

    SequenceData->ChangeFusionPatchInTrack(TrackID, NewPatch);
}

void FSequenceAssetDetails::UpdateMidiInputTracks()
{
    MidiInputTracks->ClearChildren();
    for (auto& [Index, Track] : SequenceData->TrackDisplayOptionsMap)
    {
		MidiInputTracks->AddSlot()
			.AutoHeight()
            [
				SNew(SMIDITrackControls)
					.TrackData(&Track)
                    .slotInParentID(Index)
                    .OnFusionPatchChanged(this, &FSequenceAssetDetails::OnFusionPatchChangedInTrack)

			];
	}
}
