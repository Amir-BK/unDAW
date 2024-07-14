// Copyright 2022 Jan Klimaschewski. All Rights Reserved.

#include "UnDAWSequenceEditorToolkit.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Modules/ModuleInterface.h"
#include "GlyphButton.h"
#include "UnDAWPreviewHelperSubsystem.h"
#include "Widgets/Colors/SColorPicker.h"
#include "SMidiTrackControlsWidget.h"
#include "M2SoundEdGraphSchema.h"
#include "M2SoundEdGraphNodeBaseTypes.h"
#include "SEnumCombo.h"
#include "SlateFwd.h"
#include "ITimeSlider.h"
#include "ISequencerWidgetsModule.h"
#include "Framework/Commands/GenericCommands.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "TimeSliderArgs.h"
#include "SequenceAssetEditor/DAWEditorCommands.h"
#include "SAssetDropTarget.h"

#include "Widgets/Docking/SDockTab.h"

#include "Widgets/Layout/SScaleBox.h"

void FUnDAWSequenceEditorToolkit::InitEditor(const TArray<UObject*>& InObjects)
{
	SequenceData = Cast<UDAWSequencerData>(InObjects[0]);
	GEditor->RegisterForUndo(this);
	CurrentTimestamp = CurrentTimestamp.CreateLambda([this]() { return SequenceData ? SequenceData->CurrentTimestampData : FMusicTimestamp(); });
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
	FAssetEditorToolkit::InitAssetEditor(EToolkitMode::Standalone, TSharedPtr<IToolkitHost>(), "DAWSequenceEditor", Layout, true, true, InObjects, false);
}

void FUnDAWSequenceEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(INVTEXT("unDAW Sequence Editor"));
	ISequencerWidgetsModule& SequencerWidgets = FModuleManager::Get().LoadModuleChecked<ISequencerWidgetsModule>("SequencerWidgets");
	//TSharedRef<ITimeSliderController> TimelineTimeSliderController = MakeShared<FSequencerCurveEditorTimeSliderController>(TimeSliderArgs, SequencerPtr, InSequencer->GetCurveEditor().ToSharedRef());

	//SequencerWidgets.CreateTimeSlider(TimelineTimeSliderController, false);

	InTabManager->RegisterTabSpawner("PianoRollTab", FOnSpawnTab::CreateLambda([&](const FSpawnTabArgs&)
		{
			auto DockTab = SNew(SDockTab)
				[
					SNew(SOverlay)
						+SOverlay::Slot()
						[
							SNew(SAssetDropTarget)
								//.OnAssetDropped_Lambda([this](const FAssetData& AssetData) { UE_LOG(LogTemp, Log, TEXT("That's something")); })
								//.OnAreAssetsAcceptableForDrop_Lambda([this](const TArray<FAssetData>& Assets) { return true; })
						]

						+SOverlay::Slot()
						[
							SAssignNew(PianoRollGraph, SPianoRollGraph)
								.SessionData(SequenceData)
								.Clipping(EWidgetClipping::ClipToBounds)
								//.CurrentTimestamp(SequenceData->CurrentTimestampData)
								.OnSeekEvent(OnSeekEvent)
								// .CurrentTimestamp(CurrentTimestamp)
						]
					

				];

			//PianoRollGraph->OnSeekEvent.BindLambda([this](float Seek) { OnSeekEvent.ExecuteIfBound(Seek); });

			//PianoRollGraph->Init();

			SetupPreviewPerformer();
			//PianoRollGraph->SetCurrentTimestamp(CurrentTimestamp);
			PianoRollGraph->OnMouseButtonDownDelegate.BindRaw(this, &FUnDAWSequenceEditorToolkit::OnPianoRollMouseButtonDown);

			SequenceData->OnMidiDataChanged.AddLambda([&]()
				{
					UE_LOG(LogTemp, Warning, TEXT("Midi Data Changed - did something not refresh?"));
					//PianoRollGraph->Init();
					//SetupPreviewPerformer();
					//PianoRollGraph->SetCurrentTimestamp(CurrentTimestamp);
				});

			return DockTab;
		}))
		.SetDisplayName(INVTEXT("Piano Roll"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	CreateGraphEditorWidget();

	InTabManager->RegisterTabSpawner("DAWSequenceMixerTab", FOnSpawnTab::CreateLambda([&](const FSpawnTabArgs&)
		{
			return SAssignNew(MetasoundGraphEditorBox, SDockTab)
				.Content()
				[
					MetasoundGraphEditor.ToSharedRef()
				];
		}))
		.SetDisplayName(INVTEXT("Builder Graph"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	// MetasoundGraphEditorBox->SetContent(MetasoundGraphEditor.ToSharedRef());

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObjects(TArray<UObject*>{ SequenceData });

	// FDetailsViewArgs NodeDetailsViewArgs;
   //  NodeDetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;

	 //NodeDetailsView = PropertyEditorModule.CreateDetailView(NodeDetailsViewArgs);

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

	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner("DAWSequenceMixerTab");
	InTabManager->UnregisterTabSpawner("DAWSequenceDetailsTab");
	InTabManager->UnregisterTabSpawner("PianoRollTab");
}

TSharedRef<SButton> FUnDAWSequenceEditorToolkit::GetConfiguredTransportButton(EBKTransportCommands InCommand)
{
	auto NewButton = UTransportGlyphButton::CreateTransportButton(InCommand);
	NewButton->SetOnClicked(FOnClicked::CreateLambda([&, InCommand]() { SendTransportCommand(InCommand); return FReply::Handled(); }));

	switch (InCommand)
	{
	case Play:
		NewButton->SetEnabled(TAttribute<bool>::Create([this]() { return SequenceData != nullptr && (SequenceData->PlayState == ReadyToPlay || SequenceData->PlayState == TransportPaused); }));
		NewButton->SetVisibility(TAttribute<EVisibility>::Create([this]() { return SequenceData != nullptr && ((SequenceData->PlayState == ReadyToPlay)
			|| (SequenceData->PlayState == TransportPaused)) ? EVisibility::Visible : EVisibility::Collapsed; }));
		break;
	case Stop:

		break;

	case Pause:
		NewButton->SetVisibility(TAttribute<EVisibility>::Create([this]() { return SequenceData != nullptr && (SequenceData->PlayState == TransportPlaying)
			? EVisibility::Visible : EVisibility::Collapsed; }));
		break;
	default:
		break;
	}

	return NewButton;
}

void FUnDAWSequenceEditorToolkit::OnSelectionChanged(const TSet<UObject*>& SelectedNodes)
{
	UM2SoundGraph* Graph = Cast<UM2SoundGraph>(SequenceData->M2SoundGraph);

	Graph->SelectedNodes.Empty();
	Graph->SelectedVertices.Empty();

	for (auto& Node : SelectedNodes)
	{
		if (UM2SoundEdGraphNode* SoundNode = Cast<UM2SoundEdGraphNode>(Node))
		{
			Graph->SelectedNodes.Add(SoundNode);
			Graph->SelectedVertices.Add(SoundNode->Vertex);
		}
	}
}

void FUnDAWSequenceEditorToolkit::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	//const FScopedTransaction Transaction(TEXT(""), INVTEXT("Rename Node"), NodeBeingChanged);
	//NodeBeingChanged->Modify();
	//ConcordModel->Modify();
	//NodeBeingChanged->OnRenameNode(NewText.ToString());
	//NodeDetailsView->ForceRefresh();
	//AdditionalDetailsView->ForceRefresh();
}

void FUnDAWSequenceEditorToolkit::DeleteSelectedNodes()
{
	UE_LOG(LogTemp, Warning, TEXT("Delete Selected Nodes"));
	UM2SoundGraph* Graph = Cast<UM2SoundGraph>(SequenceData->M2SoundGraph);

	for (auto& Node : Graph->SelectedNodes)
	{
		if (Node->CanUserDeleteNode())	Node->DestroyNode();
	}
}

void FUnDAWSequenceEditorToolkit::CreateGraphEditorWidget()
{
	//if (Performer && Performer->AuditionComponentRef)
	//{
	   // auto Metasound = Performer->AuditionComponentRef->GetSound();
		//FMetasoundAssetBase* MetasoundAsset = Metasound::IMetasoundUObjectRegistry::Get().GetObjectAsAssetBase(Metasound);
		//check(MetasoundAsset);

	//AdditionalGraphCommands = MakeShared<FUICommandList>();

	//AdditionalGraphCommands->MapAction(FGenericCommands::Get().Delete,)

	FGraphAppearanceInfo AppearanceInfo;
	FString CornerText = TEXT("M");
	CornerText.AppendChar(0x00B2);
	CornerText.Append(TEXT("Sound Graph"));
	AppearanceInfo.CornerText = FText::FromString(CornerText);
	AppearanceInfo.PIENotifyText = FText::FromString(TEXT("PIE is active"));

	SGraphEditor::FGraphEditorEvents GraphEvents;
	// GraphEvents.OnCreateActionMenu = SGraphEditor::FOnCreateActionMenu::CreateSP(this, &FEditor::OnCreateGraphActionMenu);
	// GraphEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FEditor::ExecuteNode);

	GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FUnDAWSequenceEditorToolkit::OnSelectionChanged);
	GraphEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FUnDAWSequenceEditorToolkit::OnNodeTitleCommitted);
	GraphEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FUnDAWSequenceEditorToolkit::OnNodeDoubleClicked);

	AdditionalGraphCommands = MakeShared<FUICommandList>();

	AdditionalGraphCommands->MapAction(FGenericCommands::Get().Delete, FExecuteAction::CreateLambda([this]() { DeleteSelectedNodes(); }));

	SAssignNew(MetasoundGraphEditor, SGraphEditor)
		//   // .OnGraphModuleReloaded_Lambda([this]() { TryAttachGraphsToPerformer(); })
		.AssetEditorToolkit(SharedThis(this))
		.AdditionalCommands(AdditionalGraphCommands)
		.Appearance(AppearanceInfo)
		.GraphEvents(GraphEvents)
		.GraphToEdit(SequenceData->M2SoundGraph);

	// }
}

void FUnDAWSequenceEditorToolkit::OnPerformerTimestampUpdated(const FMusicTimestamp& NewTimestamp)
{
	if (PianoRollGraph)    PianoRollGraph->UpdateTimestamp(NewTimestamp);
}

void FUnDAWSequenceEditorToolkit::TogglePaintingMode()
{
	bool bPaintingMode = PianoRollGraph->InputMode == EPianoRollEditorMouseMode::drawNotes;

	if (PianoRollGraph) PianoRollGraph->SetInputMode(!bPaintingMode ? EPianoRollEditorMouseMode::drawNotes : EPianoRollEditorMouseMode::Panning);
}

void FUnDAWSequenceEditorToolkit::TogglePianoTab()
{
	if (PianoRollGraph)
	{
		PianoRollGraph->PianoTabWidth.Set(*PianoRollGraph, PianoRollGraph->PianoTabWidth.Get() == 0.0f ? 50.0f : 0.0f);
	}
}

// So, we learned from flow that it's possible to make this a lot simpler by using the style class and the manu builder
// still, some 'extensions' probably need to happen here so it's not a complete waste
void FUnDAWSequenceEditorToolkit::ExtendToolbar()
{
	const FDAWEditorToolbarCommands& Commands = FDAWEditorToolbarCommands::Get();

	//transport
	ToolkitCommands->MapAction(Commands.TransportPlay, FExecuteAction::CreateLambda([this]() { SendTransportCommand(Play); }));
	ToolkitCommands->MapAction(Commands.TransportStop, FExecuteAction::CreateLambda([this]() { SendTransportCommand(Stop); }));

	//map note draw mode toggle
	ToolkitCommands->MapAction(Commands.ToggleNotePaintingMode, FExecuteAction::CreateLambda([this]() { TogglePaintingMode(); }));

	//map piano tab toggle
	ToolkitCommands->MapAction(Commands.TogglePianoTabView, FExecuteAction::CreateLambda([this]() { TogglePianoTab(); }));

	TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	CurrentPlayStateTextBox = SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(UEnum::GetValueAsString(GetCurrentPlaybackState())); })
		.Justification(ETextJustify::Center);

	ToolbarExtender->AddToolBarExtension("Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this, Commands](FToolBarBuilder& ToolbarBuilder)
			{
				auto StopButton = GetConfiguredTransportButton(Stop);
				auto PlayButton = GetConfiguredTransportButton(Play);
				auto PauseButton = GetConfiguredTransportButton(Pause);
				PlayButton->SetToolTipText(Commands.TransportPlay->GetInputText());
				ToolbarBuilder.BeginSection("Transport");

				ToolbarBuilder.AddWidget(SNew(SBox).VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							[
								SNew(SButton)
									.Text(INVTEXT("ReInit"))
									.OnClicked_Lambda([this]() { SetupPreviewPerformer(); return FReply::Handled(); })
									// .IsEnabled_Lambda([this]() { return Performer == nullptr; })
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
							+ SHorizontalBox::Slot()

							// loop control check box, monitors the value of CoreNodes.bIsLooping and calls 'SetLoopSettings' on assign value
							[
								SNew(SCheckBox)
									.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (SequenceData) SequenceData->SetLoopSettings(NewState == ECheckBoxState::Checked, SequenceData->CoreNodes.BarLoopDuration); })
									.IsChecked_Lambda([this]() -> ECheckBoxState { return SequenceData && SequenceData->CoreNodes.bIsLooping ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
									.ToolTipText(INVTEXT("Set simple loop, right now will loop the first 4 bars of the MIDI, more nuance to come\nUnfortunatley the harmonix node jump to 0 when loop is toggled"))
									.Content()
									[
										SNew(STextBlock).Text(INVTEXT("Loop"))
									]
							]
							// loop duration int numberic entry box

							+ SHorizontalBox::Slot()
							[
								SNew(SNumericEntryBox<int32>)
									.AllowSpin(true)
									.MinValue(1)
									.MaxValue(100)
									.MaxSliderValue(100)
									.MaxFractionalDigits(0)
									.MinFractionalDigits(0)
									.Value_Lambda([this]() -> int32 { return SequenceData ? SequenceData->CoreNodes.BarLoopDuration : 4; })
									.OnValueChanged_Lambda([this](int32 NewValue) { if (SequenceData) SequenceData->SetLoopSettings(true, NewValue); })

							]
					]);

				ToolbarBuilder.EndSection();

				ToolbarBuilder.BeginSection("Performer Status");
				ToolbarBuilder.AddWidget(SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					[
						CurrentPlayStateTextBox.ToSharedRef()
					]
					+ SVerticalBox::Slot()
					[
						SNew(STextBlock)
							.Text_Lambda([this]() -> FText
								{if (SequenceData)
						{
							return FText::FromString(FString::Printf(TEXT("Bar: %d, Beat: %f"), SequenceData->CurrentTimestampData.Bar, SequenceData->CurrentTimestampData.Beat));
						}
						return FText::FromString(FString::Printf(TEXT("Bar: %d, Beat: %f"), 0, 0.0f)); })
					]);

				ToolbarBuilder.EndSection();

				ToolbarBuilder.BeginSection("Graph Input");
				ToolbarBuilder.AddWidget(SNew(SEnumComboBox, StaticEnum<EPianoRollEditorMouseMode>())
					.CurrentValue_Lambda([this]() -> int32
						{
							if (!PianoRollGraph) return (int32)EPianoRollEditorMouseMode::empty;
							return (int32)PianoRollGraph->InputMode;
						})
					.OnEnumSelectionChanged_Lambda([this](int32 NewSelection, ESelectInfo::Type InSelectionInf) { if (PianoRollGraph) PianoRollGraph->SetInputMode(EPianoRollEditorMouseMode(NewSelection)); })
					.ToolTipText(Commands.ToggleNotePaintingMode->GetInputText())

				);

				//add follow cursor checkbox
				ToolbarBuilder.AddWidget(SNew(SCheckBox)
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (PianoRollGraph) PianoRollGraph->bFollowCursor = NewState == ECheckBoxState::Checked; })
					.IsChecked_Lambda([this]() -> ECheckBoxState { return PianoRollGraph && PianoRollGraph->bFollowCursor ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.Content()
					[
						SNew(STextBlock).Text(INVTEXT("Follow Cursor"))
					]
				);

				ToolbarBuilder.AddWidget(SNew(SNumericEntryBox<float>)
					.AllowSpin(true)
					.MinValue(0.0f)
					.MaxValue(127.0f)
					.MaxSliderValue(127.0f)
					.MaxFractionalDigits(2)
					.MinFractionalDigits(2)
					.Value_Lambda([this]() -> float { return PianoRollGraph ? PianoRollGraph->NewNoteVelocity : 0.0f; })
					.OnValueChanged_Lambda([this](float NewValue) { if (PianoRollGraph) PianoRollGraph->NewNoteVelocity = NewValue; }));

				ToolbarBuilder.EndSection();
			}));

	AddToolbarExtender(ToolbarExtender);
}

void FUnDAWSequenceEditorToolkit::SetupPreviewPerformer()
{
	PianoRollGraph->OnSeekEvent.Unbind();
	auto PreviewHelper = GEditor->GetEditorSubsystem<UUnDAWPreviewHelperSubsystem>();
	PreviewHelper->CreateAndPrimePreviewBuilderForDawSequence(SequenceData);

	//Performer = SequenceData->EditorPreviewPerformer;
	//Performer->OnDeleted.AddLambda([this]() { Performer = nullptr; });

	PianoRollGraph->OnSeekEvent.BindUObject(SequenceData, &UDAWSequencerData::SendSeekCommand);
}

bool FUnDAWSequenceEditorToolkit::OnAssetDraggedOver(TArrayView<FAssetData> InAssets) const
{
	UE_LOG(LogTemp, Warning, TEXT("Asset Dragged Over"));
	
	return false;
}

FReply FUnDAWSequenceEditorToolkit::OnPianoRollMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("Mouse Down"));
	return FReply::Handled();
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

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bAllowSearch = false;
	// DetailsViewArgs.
	NodeDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	NodeDetailsView->SetObjects(TArray<UObject*>{ SequenceData->M2SoundGraph });
	DetailBuilder.EditCategory("Selection")
		.AddCustomRow(FText::FromString("Selection"))
		.WholeRowContent()
		[
			NodeDetailsView.ToSharedRef()

		];
}

FSequenceAssetDetails::~FSequenceAssetDetails()
{
	SequenceData->OnMidiDataChanged.RemoveAll(this);

	MidiInputTracks.Reset();
	NodeDetailsView.Reset();
}

void FSequenceAssetDetails::UpdateMidiInputTracks()
{
	MidiInputTracks->ClearChildren();

	MidiInputTracks->AddSlot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SButton)
						.Text(INVTEXT("Add Track"))
						.OnClicked_Lambda([this]() { SequenceData->AddTrack(); return FReply::Handled(); })
				]

		];

	for (SIZE_T i = 0; i < SequenceData->M2TrackMetadata.Num(); i++)
	{
		MidiInputTracks->AddSlot()
			.AutoHeight()
			[
				SNew(SMIDITrackControls)
					.TrackData(&SequenceData->M2TrackMetadata[i])
					.slotInParentID(i)
					//.OnFusionPatchChanged(this, &FSequenceAssetDetails::OnFusionPatchChangedInTrack)
					.ConnectedGraph(SequenceData->M2SoundGraph)
					.SequencerData(SequenceData)

			];
	}
}