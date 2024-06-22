// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "SGraphPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SCanvas.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Components/PanelSlot.h"
#include "Widgets/SWidget.h"
#include "Widgets/Layout/SWrapBox.h"

#include "Widgets/Colors/SColorBlock.h"
#include <TimeSyncedPanel.h>
#include <Widgets/SCanvas.h>
#include "Widgets/Colors/SColorPicker.h"
#include <Runtime/AppFramework/Public/Widgets/Colors/SColorPicker.h>
#include "Kismet/GameplayStatics.h"
#include <Widgets/Layout/SConstraintCanvas.h>
#include "BKMusicWidgets.h"
#include "M2SoundGraphRenderer.h"
#include "EngravingSubsystem.h"
#include "Components/AudioComponent.h"
#include "HarmonixMidi/MidiFile.h"
#include "HarmonixMidi/MusicTimeSpan.h"
//#include "SMidiNoteContainer.h"

//#define PIANO_ROLL_DEBUG

BKMUSICWIDGETS_API DECLARE_LOG_CATEGORY_EXTERN(SPIANOROLLLOG, Verbose, All);

struct FLinkedMidiEvents;

struct FPianoRollKeyLines
{
public:

	FLinearColor* lineColor;
};


class BKMUSICWIDGETS_API STrackResizeArea : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STrackResizeArea)
		{}
		SLATE_ARGUMENT(TSharedPtr<ITimeSyncedPanel>, parentMidiEditor)
		SLATE_ARGUMENT(int, slotInParentID)

	SLATE_END_ARGS()

	TSharedPtr<ITimeSyncedPanel> parentMidiEditor;
	int slotInParentID;
	bool lmbDown = false;




	void Construct(const FArguments& InArgs)
	{
		parentMidiEditor = InArgs._parentMidiEditor;
		slotInParentID = InArgs._slotInParentID;
	};

	// Begin SWidget overrides.
	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(1000, 10);
	};

	TOptional<EMouseCursor::Type> GetCursor() const override
	{
		return EMouseCursor::ResizeUpDown;
	}
	// End SWidget overrides.

	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		lmbDown = true;

		return FReply::Handled().CaptureMouse(AsShared());
	};
	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		lmbDown = false;
		return FReply::Handled().ReleaseMouseCapture();
	};
	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (lmbDown) parentMidiEditor->ResizePanel(slotInParentID, MouseEvent.GetCursorDelta().Y);

		return FReply::Handled();
	};
};

DECLARE_DELEGATE(FOnInitComplete)

/**
 *
 */
class BKMUSICWIDGETS_API SPianoRollGraph : public SCompoundWidget
{
public:

	//SLATE_DECLARE_WIDGET_API(SPianoRollGraph, SCanvas)

	SLATE_BEGIN_ARGS(SPianoRollGraph)
		{}
		SLATE_ARGUMENT(FText, Text)
		SLATE_ARGUMENT(FSlateBrush, gridBrush)
		SLATE_ARGUMENT(FLinearColor, gridColor)
		SLATE_ARGUMENT(FLinearColor, accidentalGridColor)
		SLATE_ARGUMENT(FLinearColor, cNoteColor)
		SLATE_ARGUMENT(FLinearColor, noteColor)
		SLATE_ARGUMENT(float, pixelsPerBeat)
		SLATE_ARGUMENT(UDAWSequencerData*, SessionData)
		SLATE_ATTRIBUTE(FMusicTimestamp, CurrentTimestamp)
		SLATE_EVENT(FOnTransportSeekCommand, OnSeekEvent)

		//SLATE_ARGUMENT(TSharedPtr<UMIDIEditorBase>, parentMidiEditor)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

#if defined PIANO_ROLL_DEBUG
	FString debugData;
#endif

private:
	TAttribute<FMusicTimestamp> CurrentTimestamp;

	bool bIsAttributeBoundMusicTimestamp = false;

public:

	void SetCurrentTimestamp(TAttribute<FMusicTimestamp> newTimestamp);

	//FMusicTimestamp CurrentTimestamp;

	FOnTransportSeekCommand OnSeekEvent;

	bool bIsInitialized = false;

	FOnInitComplete OnInitCompleteDelegate;

	UAudioComponent* PerformanceComponent;
	UBKEditorUtilsKeyboardMappings* KeyMappings;

	TScriptInterface<IBK_MusicSceneManagerInterface> MonitoredSceneManager;

	FLinearColor gridColor = FLinearColor::White;
	FLinearColor accidentalGridColor = FLinearColor::Gray;
	FLinearColor cNoteColor = FLinearColor::Blue;

	double LastMeasuredAudioTime = 0.0;
	double CurrentTimelinePosition = 0.0;

	float NewNoteVelocity = 100.0f;

	UDAWSequencerData* SessionData;

	FLinearColor noteColor;
	FVector2f positionOffset = FVector2f(0, -125);
	float LastTickTimelinePosition;
	int32 hoveredPitch;
	//TSharedPtr<ITimeSyncedPanel> parentMidiEditor;
	//TMultiMap<int32, FLinkedNotes> Displayed
	TMap<int, bool> availableSamplesMap;

	FMeasuredGlyph CursorTest;

	TArray<int32> visibleBeats;
	TArray<int32> visibleBars;

	UObject* WorldContextObject = nullptr;

	//TEnumAsByte<EPianoRollEditorMouseMode> inputMode;

	float pixelsPerBeat = 1000.0f;
	float drawLength = 0;

	bool receivingDragUpdates = false;

	int CurrentBeatAtMouseCursor = 0;
	int CurrentBarAtMouseCursor = 0;

	SPianoRollGraph() = default;
	FString PluginDir;
	TCHAR CursorString;

	int32 ValueAtMouseCursorPostSnapping;
	int32 TickAtEndEvent;

	float horizontalZoom = 0.1f;
	float hZoomTarget = horizontalZoom;
	float verticalZoom = 0.03f;

	float zoomInterpSpeed = 10.0f;

	FVector2D localMousePosition = FVector2D::ZeroVector;
	FVector2D absMousePosition = FVector2D::ZeroVector;

	TArray<FLinearColor*> colorsArray;

	double verticalHeight = 500;
	int selectedTrackIndex = -1;
	bool someTrackIsSelected = false;

	EMusicTimeSpanOffsetUnits QuantizationGridUnit = EMusicTimeSpanOffsetUnits::Beats;
	EBKPlayState TransportPlaystate;

	FNeedReinit NeedsRinitDelegate;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance")
	FSlateFontInfo GridFont;


	TSharedPtr<SPianoRollGraph> selfSharedPtr;
	TWeakObjectPtr<UMidiFile> HarmonixMidiFile;

	//tempo and time events
	TArray<FMidiEvent> TempoEvents;
	TArray<FMidiEvent> TimeSignatureEvents;
	TArray<int> FoundChannels;

	TMap<int, FLinkedNotesTrack>* LinkedNoteDataMap;
	TArray<FLinkedMidiEvents*> CulledNotesArray;

	FLinkedMidiEvents TemporaryNote;
	FLinkedMidiEvents* SelectedNote = nullptr;
	FLinkedMidiEvents* PreviewNotePtr = nullptr;

	bool bShouldDrawNote = true;
	int LastDrawnNotePitch = INDEX_NONE;
	int32 LastDrawnNoteStartTick = INDEX_NONE;

	void InitFromMidiFile(UMidiFile* inMidiFile);
	void InitFromLinkedMidiData(TMap<int, TArray<FLinkedMidiEvents*>> inLinkedNoteDataMap);

	void Init();

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void UpdateTimestamp(FMusicTimestamp newTimestamp);

	void SetInputMode(EPianoRollEditorMouseMode newMode);

	FSongMaps* MidiSongMap;

	void AddHorizontalX(float inputX);

	void UpdateSlotsZOrder();

	void ResetCanvas();

	void RecalcGrid();

	TSharedPtr<SWrapBox> QuantizationButtons;

	TSharedPtr<SWrapBox> GetQuantizationButtons();

	FReply OnQuantizationButtonClicked(EMusicTimeSpanOffsetUnits newQuantizationUnit);

	bool bFollowCursor = true;

protected:

	friend class FUnDAWSequenceEditorToolkit;

	EPianoRollEditorMouseMode InputMode = EPianoRollEditorMouseMode::empty;

	bool FollowedCursorLastFrame = false;
	FText Text = FText::FromString(TEXT("Hello from custom widget"));

	TSharedPtr<SCanvas> RootCanvas;
	TSharedPtr<SConstraintCanvas> RootConstraintCanvas;


	//auto newSlot = RootCanvas->AddSlot();

	FSlateFontInfo Font;

	// = FVector2f::Zero();
	bool bLMBdown = false;
	bool wasLMDownLastFrame = false;
	bool isCtrlPressed = false;
	bool isShiftPressed = false;
	int32 tickAtMouse = 0;


	double pixelsPerSecond = 1000;
	double rowHeight = 200;

	//
	float MaxWidth = 999999.0f;
	float MaxHeight = 2000.0f;

	TArray<int32> cDiatonic;
	TArray<FVector2f> gridLine;
	TArray<FVector2f> vertLine;


	FSlateBrush gridBrush = FSlateBrush();

	void RecalculateSlotOffsets();


	// Begin SWidget overrides.
	//virtual FVector2D ComputeDesiredSize(float) const override;

	virtual void CacheDesiredSize(float InLayoutScaleMultiplier) override;

	TOptional<EMouseCursor::Type> GetCursor() const override
	{
		if (InputMode == EPianoRollEditorMouseMode::empty) return EMouseCursor::Default;

		return EMouseCursor::None;
	}
	// End SWidget overrides.

public:
	// SWidget interface
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry,
		const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry,
		const FKeyEvent& InKeyEvent) override;

	virtual bool SupportsKeyboardFocus() const override { return true; }

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	void DragNote(const FPointerEvent& MouseEvent);
	void StopDraggingNote();

	SPianoRollGraph* pointerToSelf;
	bool isNotePressed = false;
	int32 selectedNoteMapIndex;

	float dragDeltaAccumulator = 0;
};
