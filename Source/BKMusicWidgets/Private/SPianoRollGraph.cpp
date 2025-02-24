// Fill out your copyright notice in the Description page of Project Settings.

#include "SPianoRollGraph.h"
#include "SlateOptMacros.h"
#include "Logging/StructuredLog.h"
#include "Widgets/Input/SButton.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Algo/BinarySearch.h"

//#include <BKMusicWidgets.h>

#define LOCTEXT_NAMESPACE "PianoRollGraph"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

DEFINE_LOG_CATEGORY(SPianoRollLog);

SLATE_IMPLEMENT_WIDGET(SPianoRollGraph)
void SPianoRollGraph::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
	// SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "CurrentTimestamp", CurrentTimestamp, EInvalidateWidgetReason::None);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "PianoTabWidth", PianoTabWidth, EInvalidateWidgetReason::None);
}

SPianoRollGraph::SPianoRollGraph()
	: PianoTabWidth(*this, 0.0f)
{
	//OnMusicTimestamp.Unbind();
}

struct FEventsWithIndex
{
	FMidiEvent Event;
	int32 EventIndex;
};

// internal function that converts between the enum types
EMidiClockSubdivisionQuantization TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits InTimeSpan)
{
	switch (InTimeSpan)
	{
	case EMusicTimeSpanOffsetUnits::Ms:
		return EMidiClockSubdivisionQuantization::Beat;
		break;
	case EMusicTimeSpanOffsetUnits::Bars:
		return EMidiClockSubdivisionQuantization::Bar;
		break;
	case EMusicTimeSpanOffsetUnits::Beats:
		return EMidiClockSubdivisionQuantization::Beat;
		break;
	case EMusicTimeSpanOffsetUnits::ThirtySecondNotes:
		return	EMidiClockSubdivisionQuantization::ThirtySecondNote;
		break;
	case EMusicTimeSpanOffsetUnits::SixteenthNotes:
		return EMidiClockSubdivisionQuantization::SixteenthNote;
		break;
	case EMusicTimeSpanOffsetUnits::EighthNotes:
		return EMidiClockSubdivisionQuantization::EighthNote;
		break;
	case EMusicTimeSpanOffsetUnits::QuarterNotes:
		return EMidiClockSubdivisionQuantization::QuarterNote;
		break;
	case EMusicTimeSpanOffsetUnits::HalfNotes:
		return EMidiClockSubdivisionQuantization::HalfNote;
		break;
	case EMusicTimeSpanOffsetUnits::WholeNotes:
		return EMidiClockSubdivisionQuantization::WholeNote;
		break;
	case EMusicTimeSpanOffsetUnits::DottedSixteenthNotes:
		return EMidiClockSubdivisionQuantization::DottedSixteenthNote;
		break;
	case EMusicTimeSpanOffsetUnits::DottedEighthNotes:
		return EMidiClockSubdivisionQuantization::DottedEighthNote;
		break;
	case EMusicTimeSpanOffsetUnits::DottedQuarterNotes:
		return EMidiClockSubdivisionQuantization::DottedQuarterNote;
		break;
	case EMusicTimeSpanOffsetUnits::DottedHalfNotes:
		return EMidiClockSubdivisionQuantization::DottedHalfNote;
		break;
	case EMusicTimeSpanOffsetUnits::DottedWholeNotes:
		return EMidiClockSubdivisionQuantization::DottedWholeNote;
		break;
	case EMusicTimeSpanOffsetUnits::SixteenthNoteTriplets:
		return EMidiClockSubdivisionQuantization::SixteenthNoteTriplet;
		break;
	case EMusicTimeSpanOffsetUnits::EighthNoteTriplets:
		return EMidiClockSubdivisionQuantization::EighthNoteTriplet;
		break;
	case EMusicTimeSpanOffsetUnits::QuarterNoteTriplets:
		return EMidiClockSubdivisionQuantization::QuarterNoteTriplet;
		break;
	case EMusicTimeSpanOffsetUnits::HalfNoteTriplets:
		return EMidiClockSubdivisionQuantization::HalfNoteTriplet;
		break;
	default:
		break;
	}

	return EMidiClockSubdivisionQuantization::Beat;
}

void SPianoRollGraph::Construct(const FArguments& InArgs)

{
	//PluginDir = IPluginManager::Get().FindPlugin(TEXT("unDAW"))->GetBaseDir();

	CursorFollowAnchorPosition = InArgs._CursorFollowAnchorPosition;
	OnSeekEvent = InArgs._OnSeekEvent;
	OnMouseButtonDownDelegate = InArgs._OnMouseButtonDown;
	//OnMusicTimestamp = InArgs._OnMusicTimestamp;
	PianoTabWidth.Assign(*this, InArgs._PianoTabWidth);

	//OnMusicTimestamp.BindLambda([&](FMusicTimestamp newTimestamp) { UpdateTimestamp(newTimestamp); });
	//OnMusicTimestamp.Unbind();

	bCanSupportFocus = true;
	//we don't want to tick unless there's a midi file
	SetCanTick(false);
	//tick
	if (InArgs._SessionData) {
		SetSessionData(InArgs._SessionData);
	}
	else if (InArgs._MidiFile) {
		SetMidiFile(InArgs._MidiFile);
	}

	GridColor = InArgs._GridColor;
	AccidentalGridColor = InArgs._AccidentalGridColor;
	CNoteColor = InArgs._CNoteColor;

	//ChildSlot
	//	[
	//		SAssignNew(RootConstraintCanvas, SConstraintCanvas)
	//	];

	CursorTest = FBKMusicWidgetsModule::GetMeasuredGlyphFromHex(0xF040);

	RecalculateSlotOffsets();

	for (int i = 0; i <= 127; i++)
	{
		if (i % 12 == 0) {
			colorsArray.Add(&CNoteColor);
		}
		else {
			colorsArray.Add(UEngravingSubsystem::IsNoteInCmajor(i) ? &GridColor : &AccidentalGridColor);
		}
	}

	RecalcSubdivisions();
}
void SPianoRollGraph::RecalculateSlotOffsets()
{
	RowHeight = 200 * verticalZoom;

	vertLine.Empty();
	vertLine.Add(FVector2f(0.0f, 0.0f));
	vertLine.Add(FVector2f(0.0f, 128 * RowHeight));
}

void SPianoRollGraph::SetSessionData(UDAWSequencerData* inSessionData)
{
	if (SessionData)
	{
		if (SessionData == inSessionData) return;

		OnSeekEvent.Unbind();
	}

	if (inSessionData) {
		OnSeekEvent.BindUObject(inSessionData, &UDAWSequencerData::SendSeekCommand);
		SessionData = inSessionData;
		//this is... annoying but we'll deal with later.
		LinkedNoteDataMap = &SessionData->LinkedNoteDataMap;

		if (SessionData->HarmonixMidiFile) SetMidiFile(SessionData->HarmonixMidiFile);

		InputMode = EPianoRollEditorMouseMode::Panning;
		SetCanTick(true);
	}
	else {
		SessionData = nullptr;
		LinkedNoteDataMap = nullptr;
		SetCanTick(false);
	}
}

void SPianoRollGraph::SetMidiFile(UMidiFile* inMidiFile)
{
	MidiFile = inMidiFile;

	if (MidiFile)
	{
		SetCanTick(true);
	}
	else
	{
		SetCanTick(false);
	}
}

void SPianoRollGraph::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (SessionData == nullptr) return;

	auto* MidiSongMap = MidiFile->GetSongMaps();

	const auto tick = MidiSongMap->CalculateMidiTick(SessionData->CurrentTimestampData, EMidiClockSubdivisionQuantization::None);
	const auto CurrentTimeMiliSeconds = MidiSongMap->TickToMs(tick);

	//timeline is in miliseconds
	CurrentTimelinePosition = CurrentTimeMiliSeconds * .001f;

	if (bFollowCursor && SessionData->PlayState == EBKPlayState::TransportPlaying)
	{

		float DesiredCursorPosition = (GetCachedGeometry().GetLocalSize().X - PianoTabWidth.Get()) * CursorFollowAnchorPosition - PositionOffset.X;
		float LocalSpacePlayBackPosition = CurrentTimeMiliSeconds * HorizontalZoom; //+ positionOffset.X;

		PositionOffset.X = -LocalSpacePlayBackPosition;// + DesiredCursorPosition;

		float BeginningOfScreen = -PositionOffset.X;

	}

	//this is the zoom smoothing function, it is not stable.
	if (HorizontalZoom != hZoomTarget)
	{
		//here we keep our zoom position close to where the mouse was, I need to also add interpolation towards the center
		//and make this 2d instead of 1d

		float preZoomTimeAtMouse = localMousePosition.X / HorizontalZoom;

		HorizontalZoom = FMath::Lerp<float>(HorizontalZoom, hZoomTarget, 0.2f);

		float postZoomTimeAtMouse = localMousePosition.X / HorizontalZoom;
		float timeDelta = postZoomTimeAtMouse - preZoomTimeAtMouse;
		PositionOffset.X += timeDelta * HorizontalZoom;
		PositionOffset.X = FMath::Min(PositionOffset.X, 0.0f);

		//verticalZoom = FMath::Clamp(verticalZoom, 0.01, 2.0);
		RecalculateSlotOffsets();
	}

	if (verticalZoom != vZoomTarget)
	{
		//here we keep our zoom position close to where the mouse was, I need to also add interpolation towards the center
		//and make this 2d instead of 1d

		float preZoomTimeAtMouse = localMousePosition.Y / verticalZoom;

		verticalZoom = FMath::Lerp<float>(verticalZoom, vZoomTarget, 0.2f);

		float postZoomTimeAtMouse = localMousePosition.Y / verticalZoom;
		float timeDelta = postZoomTimeAtMouse - preZoomTimeAtMouse;
		PositionOffset.Y += timeDelta * verticalZoom;
		PositionOffset.Y = FMath::Min(PositionOffset.Y, 0.0f);

		verticalZoom = FMath::Clamp(verticalZoom, 0.01, 2.0);
		RecalculateSlotOffsets();
	}

	PositionOffset.X = FMath::Min(PositionOffset.X, 0.0f);

	//update cursor and the such
	if (!receivingDragUpdates)
	{
		tickAtMouse = MidiSongMap->MsToTick((localMousePosition.X - PianoTabWidth.Get()) / HorizontalZoom);
		CurrentBeatAtMouseCursor = MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse).Beat;
		CurrentBarAtMouseCursor = MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse).Bar;
		//int toTickBar = MidiSongMap->GetBarMap().MusicTimestampBarBeatTickToTick(bar, beat, 0);
		//int PrevBeatTick = toTickBar + (MidiSongMap->GetTicksPerQuarterNote() * (beat - 1));//+ MidiSongMap->GetTicksPerQuarterNote() * MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), toTickBar);
		ValueAtMouseCursorPostSnapping = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse), TimeSpanToSubDiv(QuantizationGridUnit));
		if (QuantizationGridUnit == EMusicTimeSpanOffsetUnits::Ms) ValueAtMouseCursorPostSnapping = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse), EMidiClockSubdivisionQuantization::None);
	}

	//we need to update hovered pitch every tick
	HoveredPitch = 127 - FMath::Floor(localMousePosition.Y / RowHeight);

	if (InputMode == EPianoRollEditorMouseMode::drawNotes && SessionData->SelectedTrackIndex != INDEX_NONE)
	{
		if (!isCtrlPressed && HoveredPitch != LastDrawnNotePitch || LastDrawnNoteStartTick != ValueAtMouseCursorPostSnapping)
		{
			//calculate end tick, for now just add the quantization value to the start stick
			auto NumTicksPerSubdivision = MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), ValueAtMouseCursorPostSnapping);

			FLinkedMidiEvents newNote = FLinkedMidiEvents();
			newNote.NoteVelocity = NewNoteVelocity;
			auto TrackMetadata = SessionData->GetTrackMetadata(SessionData->SelectedTrackIndex);
			newNote.Pitch = HoveredPitch;
			newNote.TrackId = SessionData->SelectedTrackIndex;
			newNote.ChannelId = TrackMetadata.ChannelIndexRaw;
			newNote.StartTick = ValueAtMouseCursorPostSnapping;
			newNote.EndTick = ValueAtMouseCursorPostSnapping + NumTicksPerSubdivision;
			newNote.CalculateDuration(MidiSongMap);
			LastDrawnNotePitch = HoveredPitch;
			LastDrawnNoteStartTick = ValueAtMouseCursorPostSnapping;

			TemporaryNote = newNote;
			PreviewNotePtr = &TemporaryNote;

			// to fix note creation, put this loop here, but deleting won't work, it shouldn't be called for here anyway...
			//if lmb down commit note
		}

	}

	if (bLMBdown)
	{
		if (isCtrlPressed && SelectedNote != nullptr) {
			SessionData->DeleteLinkedMidiEvent(*SelectedNote);
			SelectedNote = nullptr;
			UE_LOG(LogTemp, Log, TEXT("Mouse Down! should have tried to delete note!"))
		}

		if (PreviewNotePtr && !isCtrlPressed)
		{
			UE_LOG(LogTemp, Log, TEXT("Mouse Down! should have tried to create note!"))
				SessionData->AddLinkedMidiEvent(TemporaryNote);
			PreviewNotePtr = nullptr;
		}

		//SelectedNote = nullptr;
	}
}

void SPianoRollGraph::UpdateTimestamp(FMusicTimestamp newTimestamp)
{
	checkNoEntry();

}
void SPianoRollGraph::SetInputMode(EPianoRollEditorMouseMode newMode)
{
	InputMode = newMode;
	switch (newMode) {
	case EPianoRollEditorMouseMode::drawNotes:
		if (isCtrlPressed)
		{
			CursorTest = FBKMusicWidgetsModule::GetMeasuredGlyphFromHex(0xF014);
			PreviewNotePtr = nullptr;
		}
		else {
			CursorTest = FBKMusicWidgetsModule::GetMeasuredGlyphFromHex(0xF040);
		}
		break;

	case EPianoRollEditorMouseMode::Panning:
		CursorTest = FMeasuredGlyph{ TCHAR(0xF05B), 0.0f, 0.0f };
		break;

		//case EPianoRollEditorMouseMode::pan:
		//	CursorTest = FMeasuredGlyph{ TCHAR(0xF047), 0.0f, 0.0f };
		//	break;
	case EPianoRollEditorMouseMode::seek:
		CursorTest = FMeasuredGlyph{ TCHAR(0xF028), 0.0f, 0.0f };
		break;
	default:
		CursorTest = FBKMusicWidgetsModule::GetMeasuredGlyphFromHex(0xF00E);
		break;
	}
}
void SPianoRollGraph::AddHorizontalX(float inputX)
{
	PositionOffset.X += inputX;
	PositionOffset = FVector2D::Min(PositionOffset, FVector2D::ZeroVector);

	RecalcGrid();
}

void SPianoRollGraph::CacheDesiredSize(float InLayoutScaleMultiplier) //Super::CacheDesiredSize(InLayoutScaleMultiplier)
{
	RecalcGrid();
}
inline TOptional<EMouseCursor::Type> SPianoRollGraph::GetCursor() const
{
	//if (InputMode == EPianoRollEditorMouseMode::empty || InputMode == EPianoRollEditorMouseMode::Panning) return EMouseCursor::Default;

	return CursorType;
}
;

void SPianoRollGraph::RecalcGrid()
{
	if (SessionData == nullptr) return;
	using namespace UnDAW;

	auto* SongsMap = MidiFile->GetSongMaps();
	//after much figuring out, this is the code that generates the grid based on quantization size
	VisibleBeats.Empty();
	float LeftMostTick = SongsMap->MsToTick(-PositionOffset.X / HorizontalZoom);
	float RightMostTick = SongsMap->MsToTick((GetCachedGeometry().GetLocalSize().X - PositionOffset.X) / HorizontalZoom);

	VisibleBars.Empty();
	VisibleSubdivisions.Empty();

// can probably just leave it as time, rather than calculate back and forth...
	float barTick = LeftMostTick;
	while (!SongsMap->GetBarMap().IsEmpty() && barTick <= RightMostTick)
	{
		//MidiSongMap->GetBarMap()
		auto BarTick = SongsMap->CalculateMidiTick(SongsMap->GetBarMap().TickToMusicTimestamp(barTick), TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars));
		VisibleBars.Add(BarTick);
		//visibleBars.Add(MidiSongMap->GetBarMap().MusicTimestampBarToTick(bars));
		FMusicalGridPoint BarGridPoint = { };
		//GridPointMap.Add(BarTick, BarGridPoint);

		barTick += SongsMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars), barTick);
	}


	//populate beats array
	float subDivTick = LeftMostTick;
	while (!SongsMap->GetBarMap().IsEmpty() && subDivTick <= RightMostTick)
	{
		VisibleBeats.Add(SongsMap->CalculateMidiTick(SongsMap->GetBarMap().TickToMusicTimestamp(subDivTick), TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Beats)));
		subDivTick += SongsMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Beats), subDivTick);
	}

	//print quantization grid unit 
	

	// if we're not in beats mode, we need to calculate the subdivisions
	subDivTick = LeftMostTick;
	if(QuantizationGridUnit != EMusicTimeSpanOffsetUnits::Beats)
	{
		int SubDivCount = 1;
		while (!SongsMap->GetBarMap().IsEmpty() && subDivTick <= RightMostTick)
		{
		//VisibleSubdivisions.Add(MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(subDivTick), TimeSpanToSubDiv(QuantizationGridUnit)));
		if (GridPointMap.Contains(subDivTick))
		{
			SubDivCount = 1;
		}
		else {
			//GridPointMap.Add(subDivTick, { EGridPointType::Subdivision, ++SubDivCount });
		}
		
		subDivTick += SongsMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), subDivTick);
		

		}
	}




	//cull notes
	CulledNotesArray.Empty();

	if (!LinkedNoteDataMap) return;

	for (auto& track : SessionData->Tracks)
	{
		for (auto& clip : track.LinkedNotesClips)
		{
			for (auto& note : clip.LinkedNotes)
			{
				bool NoteInRightBound = note.StartTick < RightMostTick;

				if (note.EndTick + clip.StartTick >= LeftMostTick)
				{
					CulledNotesArray.Add(&note);

				}
			}
		}
		

	}

	CulledNotesArray.Sort([](const FLinkedMidiEvents& A, const FLinkedMidiEvents& B) { return A.StartTick < B.StartTick; });

	gridLine.Empty(2);
	//horizontal line
	gridLine.Add(FVector2f(-1 * PositionOffset.X, 0.0f));
	gridLine.Add(FVector2f(DrawLength, 0.0f));

	vertLine.Empty(2);
	//vertical line
	vertLine.Add(FVector2f(0.0f, 0.0f));
	vertLine.Add(FVector2f(0.0f, 127 * RowHeight));
}

void SPianoRollGraph::RecalcSubdivisions()
{
	if (SessionData == nullptr) return;
	using namespace UnDAW;

	const auto& SongsMap = SessionData->HarmonixMidiFile->GetSongMaps();
	UE_LOG(SPianoRollLog, Log, TEXT("Quantization Grid Unit: %s"), *UEnum::GetValueAsString(QuantizationGridUnit));
	GridPointMap.Empty();

	// this time we're populating the entire grid point map
	const float FirstTickOfFirstBar = SongsMap->GetBarMap().BarBeatTickIncludingCountInToTick(1,1, 0);
	const float LastTickOfLastBar = SessionData->HarmonixMidiFile->GetLastEventTick();

	int32 BarCount = 1;
	float BarTick = 0;
	while (BarTick <= LastTickOfLastBar)
	{
		//VisibleBars.Add(MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(BarTick), TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars)));
		GridPointMap.Add(BarTick, { EGridPointType::Bar, BarCount++, 1, 1 });
		BarTick += SongsMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars), BarTick);
	}

	int8 BeatCount = 0;
	float BeatTick = 0;
	int32 CurrentBar = 0;
	while (BeatTick <= LastTickOfLastBar)
	{
		
		//VisibleBeats.Add(MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(BeatTick), TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Beats));
		if (GridPointMap.Contains(BeatTick))
		{
			CurrentBar = GridPointMap[BeatTick].Bar;
			BeatCount = 1;
		}
		else {
			GridPointMap.Add(BeatTick, { EGridPointType::Beat, CurrentBar, ++BeatCount });
		}

		BeatTick += SongsMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Beats), BeatTick);
	}



	int32 subDivTick = 0;
	int8 subDivCount = 0;
	int8 CurrentBeat = 1;
	while (subDivTick <= LastTickOfLastBar)
	{
		//VisibleSubdivisions.Add(MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(subDivTick), TimeSpanToSubDiv(QuantizationGridUnit)));
		if (GridPointMap.Contains(subDivTick))
		{
			if (GridPointMap[subDivTick].Type == EGridPointType::Bar)
			{
			subDivCount = 1;
			CurrentBar = GridPointMap[subDivTick].Bar;
			}
			else {
				//this is a beat
				CurrentBeat = GridPointMap[subDivTick].Beat;
				GridPointMap[subDivTick].Subdivision = ++subDivCount;
			}

		}
		else {
			GridPointMap.Add(subDivTick, { EGridPointType::Subdivision,CurrentBar, CurrentBeat, ++subDivCount });
		}

		subDivTick += SongsMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), subDivTick);
	}


	



}

TSharedPtr<SWrapBox> SPianoRollGraph::GetQuantizationButtons()
{

	return SNew(SWrapBox)
		//.PreferredSize(FVector2D(500.0f, 100.0f))
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Ms"))
				//.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::Ms)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Bars"))
				//.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::Bars)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Beats"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::Beats)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Quarter Notes"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::QuarterNotes)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Eighth Notes"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::EighthNotes)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Sixteenth Notes"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::SixteenthNotes)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Thirty Second Notes"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::ThirtySecondNotes)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Half Notes"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::HalfNotes)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Whole Notes"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::WholeNotes)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Dotted Quarter Notes"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::DottedQuarterNotes)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Dotted Eighth Notes"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::DottedEighthNotes)
		];

	return QuantizationButtons;
}

FReply SPianoRollGraph::OnQuantizationButtonClicked(EMusicTimeSpanOffsetUnits newQuantizationUnit)
{
	QuantizationGridUnit = newQuantizationUnit;
	RecalcGrid();

	return FReply::Handled();
}

FLinkedMidiEvents CreateNoteAtMousePosition(FVector2D mousePosition, FSongMaps* MidiSongMap, EMusicTimeSpanOffsetUnits QuantizationGridUnit)
{
	FLinkedMidiEvents newNote;
	newNote.StartTick = MidiSongMap->MsToTick(mousePosition.X);
	newNote.EndTick = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(newNote.StartTick), EMidiClockSubdivisionQuantization::Bar);
	newNote.Pitch = mousePosition.Y;
	newNote.TrackId = 1;
	newNote.ChannelId = 1;
	newNote.CalculateDuration(MidiSongMap);

	return newNote;
}

FReply SPianoRollGraph::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{

	const bool bIsLeftMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
	const bool bIsRightMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
	const bool bIsMiddleMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton;
	const bool bIsRightMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
	const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
	const bool bIsMiddleMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton);

	FReply Reply = FReply::Unhandled();
	if (OnMouseButtonDownDelegate.IsBound())
	{
		Reply = OnMouseButtonDownDelegate.Execute(MyGeometry, MouseEvent);
	}

	if (bIsLeftMouseButtonEffecting)
	{
		//test marquee
		// if we're clicking in the top 45 pixels (MAGIC NUMBERS) this is a seek click, for now print log
		//auto localMousePosition = GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - PositionOffset;
		if (GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).Y < 45.0f)
		{
			OnSeekEvent.ExecuteIfBound(MidiFile->GetSongMaps()->TickToMs(ValueAtMouseCursorPostSnapping) * .001f);
		}


	}

	if (bIsRightMouseButtonDown)
	{
		CursorType = EMouseCursor::GrabHandClosed;
	}


	if (HasKeyboardFocus())
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !bLMBdown)
		{
			bLMBdown = true;

			//return OnMouseMove(MyGeometry, MouseEvent);
			//return FReply::Unhandled().CaptureMouse(AsShared());
		}
	}

	return Reply;
}
FReply SPianoRollGraph::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	
	const bool bIsLeftMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
	const bool bIsRightMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
	const bool bIsMiddleMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton;
	const bool bIsRightMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
	const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
	const bool bIsMiddleMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton);

	if(bIsRightMouseButtonEffecting)
	{
		CursorType = EMouseCursor::Default;
		return FReply::Unhandled().ReleaseMouseCapture();
	}
	
	
	
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bLMBdown = false;
		return FReply::Unhandled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}
FReply SPianoRollGraph::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Unhandled();
}
FReply SPianoRollGraph::OnMouseWheel(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (isCtrlPressed) {
		if (InMouseEvent.GetWheelDelta() >= 0.1)
		{
			hZoomTarget *= 1.1f;
			//hZoomTarget = FMath::Lerp<float>(hZoomTarget, 2.0f, 0.05f);
		}
		else {
			hZoomTarget *= 0.9f;
			//hZoomTarget = FMath::Lerp<float>(hZoomTarget, 0.02f, 0.05f);
		}

		const FVector2D WidgetSpaceCursorPos = InMyGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());


		absMousePosition = InMouseEvent.GetScreenSpacePosition();
		return FReply::Handled();
	}
	else if (isShiftPressed)
	{
		if (InMouseEvent.GetWheelDelta() >= 0.1)
		{
			vZoomTarget *= 1.1f;
		}
		else {
			vZoomTarget *= 0.9f;
		}

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPianoRollGraph::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//if(!bIsInitialized) return FReply::Unhandled();

	const bool bIsRightMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
	const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
	const bool bIsMiddleMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton);
	const FModifierKeysState ModifierKeysState = FSlateApplication::Get().GetModifierKeys();

	auto abs = MouseEvent.GetScreenSpacePosition();
	localMousePosition = GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - PositionOffset;
	HoveredPitch = 127 - FMath::Floor(localMousePosition.Y / RowHeight);

	if (MidiFile == nullptr) return FReply::Unhandled();

	auto* MidiSongMap = MidiFile->GetSongMaps();

	tickAtMouse = MidiSongMap->MsToTick(localMousePosition.X / HorizontalZoom);
	//hoveredNotePitch = -1;

	SelectedNote = nullptr;
	CulledNotesArray.FindByPredicate([&](FLinkedMidiEvents* note) {
		if (tickAtMouse >= note->StartTick && tickAtMouse <= note->EndTick)
		{
			if (note->Pitch == HoveredPitch)
			{
				SelectedNote = note;
				return true;
			}
		}
		;
		return false;
		});

	//ugly and hacky, we have two maps to traverse, I don't think this makes anything more efficient
	//in the coming refactor we will push all notes to the midi file as they come
	//perhaps compartmantalizing each track to its own file, need some tests
	SessionData->PendingLinkedMidiNotesMap.FindByPredicate([&](FLinkedMidiEvents& note) {
		if (tickAtMouse >= note.StartTick && tickAtMouse <= note.EndTick)
		{
			if (note.Pitch == HoveredPitch)
			{
				SelectedNote = &note;
				return true;
			}
		}
		;
		return false;
		});

	if (bIsRightMouseButtonDown)
	{

		PositionOffset.Y += MouseEvent.GetCursorDelta().Y;
		AddHorizontalX(MouseEvent.GetCursorDelta().X);
		return FReply::Handled().CaptureMouse(AsShared());
	}

	if (bIsLeftMouseButtonDown) {
		
		if (GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).Y < 45.0f)
		{

			OnSeekEvent.ExecuteIfBound(MidiFile->GetSongMaps()->TickToMs(ValueAtMouseCursorPostSnapping) * .001f);
		}
		return FReply::Handled().CaptureMouse(AsShared());
	}

	return FReply::Unhandled();
}

EMusicTimeSpanOffsetUnits IncQuantizationSnap(EMusicTimeSpanOffsetUnits InQuantization)
{
	switch (InQuantization)
	{
	case EMusicTimeSpanOffsetUnits::Ms:
		return EMusicTimeSpanOffsetUnits::Bars;
		break;
	case EMusicTimeSpanOffsetUnits::Bars:
		return EMusicTimeSpanOffsetUnits::Beats;
		break;
	case EMusicTimeSpanOffsetUnits::Beats:
		return EMusicTimeSpanOffsetUnits::QuarterNotes;
		break;
	case EMusicTimeSpanOffsetUnits::QuarterNotes:
		return EMusicTimeSpanOffsetUnits::EighthNotes;
		break;
	case EMusicTimeSpanOffsetUnits::EighthNotes:
		return EMusicTimeSpanOffsetUnits::SixteenthNotes;
		break;
	case EMusicTimeSpanOffsetUnits::SixteenthNotes:
		return EMusicTimeSpanOffsetUnits::ThirtySecondNotes;
		break;
	case EMusicTimeSpanOffsetUnits::ThirtySecondNotes:
		return EMusicTimeSpanOffsetUnits::HalfNotes;
		break;
	case EMusicTimeSpanOffsetUnits::HalfNotes:
		return EMusicTimeSpanOffsetUnits::WholeNotes;
		break;
	case EMusicTimeSpanOffsetUnits::WholeNotes:
		return EMusicTimeSpanOffsetUnits::DottedQuarterNotes;
		break;
	case EMusicTimeSpanOffsetUnits::DottedQuarterNotes:
		return EMusicTimeSpanOffsetUnits::DottedEighthNotes;
		break;
	case EMusicTimeSpanOffsetUnits::DottedEighthNotes:
		return EMusicTimeSpanOffsetUnits::Ms;
		break;
	default:
		return EMusicTimeSpanOffsetUnits::Ms;
		break;
	}
}

EMusicTimeSpanOffsetUnits DecQuantizationSnap(EMusicTimeSpanOffsetUnits InQuantizationSnap)
{
	switch (InQuantizationSnap)
	{
	case EMusicTimeSpanOffsetUnits::Ms:
		return EMusicTimeSpanOffsetUnits::DottedEighthNotes;
		break;
	case EMusicTimeSpanOffsetUnits::Bars:
		return EMusicTimeSpanOffsetUnits::Ms;
		break;
	case EMusicTimeSpanOffsetUnits::Beats:
		return EMusicTimeSpanOffsetUnits::Bars;

		break;
	case EMusicTimeSpanOffsetUnits::QuarterNotes:
		return EMusicTimeSpanOffsetUnits::Beats;
		break;
	case EMusicTimeSpanOffsetUnits::EighthNotes:
		return EMusicTimeSpanOffsetUnits::QuarterNotes;
		break;
	case EMusicTimeSpanOffsetUnits::SixteenthNotes:
		return EMusicTimeSpanOffsetUnits::EighthNotes;
		break;
	case EMusicTimeSpanOffsetUnits::ThirtySecondNotes:
		return EMusicTimeSpanOffsetUnits::SixteenthNotes;
		break;
	case EMusicTimeSpanOffsetUnits::HalfNotes:
		return EMusicTimeSpanOffsetUnits::ThirtySecondNotes;
		break;
	case EMusicTimeSpanOffsetUnits::WholeNotes:
		return EMusicTimeSpanOffsetUnits::HalfNotes;
		break;
	case EMusicTimeSpanOffsetUnits::DottedQuarterNotes:
		return EMusicTimeSpanOffsetUnits::WholeNotes;
		break;
	case EMusicTimeSpanOffsetUnits::DottedEighthNotes:
		return EMusicTimeSpanOffsetUnits::DottedQuarterNotes;
		break;
	default:
		return EMusicTimeSpanOffsetUnits::Ms;
		break;
	}
}

FReply SPianoRollGraph::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	
	return FReply::Unhandled();
}
FReply SPianoRollGraph::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	isCtrlPressed = InKeyEvent.IsControlDown();
	isShiftPressed = InKeyEvent.IsShiftDown();

	SetInputMode(InputMode);

	return FReply::Unhandled();
}

int32 SPianoRollGraph::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), CursorTest, ESlateDrawEffect::None, FLinearColor::White);
	//if (!bIsInitialized) return LayerId;

	// bad stupid code, this should happen in the tick event

	bool bShowPianoTab = true;
	bool bShowTimeline = true;

	float TimelineMargin = 42.0f;

	auto MarginVector = FVector2D(PianoTabWidth.Get(), TimelineMargin);
	auto PaintPosVector = PositionOffset + MarginVector;

	auto OffsetGeometryChild = AllottedGeometry.MakeChild(AllottedGeometry.GetLocalSize(), FSlateLayoutTransform(1.0f, (FVector2d)PaintPosVector));
	//auto OffsetVertGeometryChild = AllottedGeometry.MakeChild((FVector2d)(0, -positionOffset.Y), AllottedGeometry.GetLocalSize(), 1.0f);

	//draw background grid
	if (&gridBrush != nullptr)
	{
		for (int i = 0; i <= 127; i++)
		{
			//FLinearColor gri
			FSlateDrawElement::MakeBox(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(0, RowHeight * (127 - i)))),
				&gridBrush,
				ESlateDrawEffect::None,
				!someTrackIsSelected || AvailableSamplesMap.Find(i) ? *colorsArray[i] : colorsArray[i]->operator+(FLinearColor(0.01f, 0.00f, 0.00f, 0.01f))
			);
		}

	};

	if (SessionData == nullptr) return LayerId;

	auto* MidiSongMap = MidiFile->GetSongMaps();


	//mouse crosshairs
	FLinearColor trackColor = SessionData->SelectedTrackIndex != INDEX_NONE ? SessionData->GetTrackMetadata(SessionData->SelectedTrackIndex).TrackColor : FLinearColor::White;

	int toTickBar = MidiSongMap->GetBarMap().MusicTimestampBarBeatTickToTick(CurrentBarAtMouseCursor, CurrentBeatAtMouseCursor, 0);
	int PrevBeatTick = toTickBar + (MidiSongMap->GetTicksPerQuarterNote() * (CurrentBeatAtMouseCursor - 1));//+ MidiSongMap->GetTicksPerQuarterNote() * MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), toTickBar);
	using namespace UnDAW;

	

	//paint hovered note
	if (SelectedNote != nullptr)
	{
		//	FSlateDrawElement::bord

		// get selected note color based on metadata and produce negative color
		auto& SelectedNoteTrackColor = SessionData->GetTrackMetadata(SelectedNote->TrackId).TrackColor;
		auto colorNegative = FLinearColor::White.operator-(SelectedNoteTrackColor);

		auto& note = SelectedNote;
		FSlateDrawElement::MakeBox(OutDrawElements,
			LayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D((note->Duration + 20) * HorizontalZoom, RowHeight + (50 * verticalZoom)), FSlateLayoutTransform(1.0f, FVector2D((note->StartTime - 5.0f) * HorizontalZoom, (-25.0f * verticalZoom) + RowHeight * (127 - note->Pitch)))),
			&gridBrush,
			ESlateDrawEffect::None,
			colorNegative.CopyWithNewOpacity(0.5f)
		);
	}

	for (auto& note : CulledNotesArray)
	{
		TArray<FSlateGradientStop> GradientStops = { FSlateGradientStop(FVector2D(0,0), SessionData->GetTrackMetadata(note->TrackId).TrackColor) };
		FSlateDrawElement::MakeGradient(OutDrawElements,
			LayerId + note->TrackId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(note->Duration * HorizontalZoom, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(note->StartTime * HorizontalZoom, RowHeight * (127 - note->Pitch)))),
			GradientStops, EOrientation::Orient_Horizontal, ESlateDrawEffect::None,
			FVector4f::One() * 2.0f
		);
	}

	for (const auto& note : SessionData->PendingLinkedMidiNotesMap)
	{
		TArray<FSlateGradientStop> GradientStops = { FSlateGradientStop(FVector2D(0,0), SessionData->GetTrackMetadata(note.TrackId).TrackColor) };
		FSlateDrawElement::MakeGradient(OutDrawElements,
			LayerId + note.TrackId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(note.Duration * HorizontalZoom, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(note.StartTime * HorizontalZoom, RowHeight * (127 - note.Pitch)))),
			GradientStops, EOrientation::Orient_Horizontal, ESlateDrawEffect::None,
			FVector4f::One() * note.cornerRadius
		);
	}

	// anything we want over the notes should have higher layer ID...
	int PostNotesLayerID = LayerId + SessionData->M2TrackMetadata.Num() + 1;

	//draw preview note
	if (PreviewNotePtr != nullptr)
	{
		auto& note = PreviewNotePtr;
		FSlateDrawElement::MakeBox(OutDrawElements,
			PostNotesLayerID,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(note->Duration * HorizontalZoom, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(note->StartTime * HorizontalZoom, RowHeight * (127 - note->Pitch)))),
			&gridBrush,
			ESlateDrawEffect::None,
			trackColor.CopyWithNewOpacity(0.5f)
		);
	}


	if (InputMode == EPianoRollEditorMouseMode::drawNotes)
	{
		FSlateDrawElement::MakeText(OutDrawElements,
			PostNotesLayerID,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(1.0f, 1.0f), FSlateLayoutTransform(1.0f, localMousePosition - FVector2D(0.0f, CursorTest.measuredY))),
			&CursorTest.glyph,
			FSlateFontInfo(PluginDir / TEXT("Resources/UtilityIconsFonts/icons.ttf"), 24),
			ESlateDrawEffect::None,
			FLinearColor::White);
	}

	//draw piano roll, overlayed on the original geometry (not the offset geometry), let's start with drawing a big gray background rectangle
	auto PianorollLayerId = PostNotesLayerID++ + 30; //lazy...
	
	for (int i = 0; i <= 127; i++)
	{
		bool isSelectedNote = i == HoveredPitch;
		//float opacity = (float)0.7f * (127.0f - FMath::Abs(i - hoveredPitch) * 12) / 127.0f;
		FLinearColor TracksColor = UEngravingSubsystem::IsNoteInCmajor(i) ? FLinearColor::White : FLinearColor::Black;

		FSlateDrawElement::MakeBox(OutDrawElements,
			PianorollLayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MarginVector.X, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(-PaintPosVector.X, RowHeight * (127 - i)))),
			&gridBrush,
			ESlateDrawEffect::None,
			TracksColor
		);

		TArray<FVector2D> HorizontalLinePoints;
		HorizontalLinePoints.Add(FVector2D(0, RowHeight));
		HorizontalLinePoints.Add(FVector2D(MarginVector.X, RowHeight));
		//also draw black lines for the white keys

		FSlateDrawElement::MakeLines(OutDrawElements,
			PianorollLayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(0.0f, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(-PaintPosVector.X, RowHeight * (127 - i)))),
			HorizontalLinePoints,
			ESlateDrawEffect::None,
			FLinearColor::Black,
			false,
			2.0f);
	}

	for (const auto& [MetadataIndex, NoteNumber] : SessionData->CurrentlyActiveNotes)
	{
		FSlateDrawElement::MakeBox(OutDrawElements,
			PianorollLayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MarginVector.X, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(-PaintPosVector.X, RowHeight * (127 - NoteNumber)))),
			&gridBrush,
			ESlateDrawEffect::None,
			SessionData->GetTrackMetadata(MetadataIndex).TrackColor
		);
	}

	//FSlateDrawElement::MakeBox(OutDrawElements,
	//	PostNotesLayerID,
	//	AllottedGeometry.ToPaintGeometry(FVector2D(MarginVector.X, AllottedGeometry.GetLocalSize().Y), FSlateLayoutTransform(1.0f, FVector2D(0.0f, 0.0f))),
	//	&gridBrush,
	//	ESlateDrawEffect::None,
	//	FLinearColor::White
	//);

	if (InputMode == EPianoRollEditorMouseMode::drawNotes && SessionData->SelectedTrackIndex != INDEX_NONE)// && someTrackIsSelected && parentMidiEditor->getCurrentInputMode() == EPianoRollEditorMouseMode::drawNotes)
	{
		//FLinearColor trackColor = SessionData->GetTracksDisplayOptions(SessionData->SelectedTrackIndex).trackColor;
		FLinearColor offTracksColor = trackColor.Desaturate(0.5);
		FLinearColor colorNegative = FLinearColor::White.operator-(trackColor);

		for (int i = 0; i <= 127; i++)
		{
			bool isSelectedNote = i == HoveredPitch;
			float opacity = (float)0.7f * (127.0f - FMath::Abs(i - HoveredPitch) * 12) / 127.0f;

			FSlateDrawElement::MakeBox(OutDrawElements,
				PostNotesLayerID++,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(-PaintPosVector.X, RowHeight * (127 - i)))),
				&gridBrush,
				ESlateDrawEffect::None,
				offTracksColor.CopyWithNewOpacity(opacity)
			);

			FSlateDrawElement::MakeText(OutDrawElements,
				PostNotesLayerID++,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(150, RowHeight), FSlateLayoutTransform(1.0f, FVector2D((double)-PaintPosVector.X, RowHeight * (127 - i)))),
				FText::FromString(FString::Printf(TEXT("%d"), i)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 9),
				ESlateDrawEffect::None,
				isSelectedNote ? colorNegative.CopyWithNewOpacity(1.0f) : colorNegative.CopyWithNewOpacity(opacity)
			);
		}
	}

	int timelineLayerID = PostNotesLayerID++;

	//paint black box 50 units height as background for timeline
	FSlateDrawElement::MakeBox(OutDrawElements,
		timelineLayerID,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, TimelineMargin), FSlateLayoutTransform(1.0f, FVector2D(0.0f, -PaintPosVector.Y))),
		&gridBrush,
		ESlateDrawEffect::None,
		FLinearColor::Black
	);

	LayerId = timelineLayerID + 1;
	//draw subdivisions
	for (const auto& [Tick, GridPoint] : GridPointMap)
	{
		FLinearColor LineColor;

		//ugly as heck but probably ok for now
		switch (GridPoint.Type)
		{
		case EGridPointType::Bar:
			//draw bar number
			FSlateDrawElement::MakeText(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * HorizontalZoom, -PaintPosVector.Y))),
				FText::FromString(FString::FromInt(GridPoint.Bar)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 14),
				ESlateDrawEffect::None,
				FLinearColor::White
			);

			//draw beat number
			FSlateDrawElement::MakeText(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * HorizontalZoom, -PaintPosVector.Y + 18))),
				FText::FromString(FString::FromInt(GridPoint.Beat)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
				ESlateDrawEffect::None,
				FLinearColor::White
			);

			//draw subdivision number
			FSlateDrawElement::MakeText(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * HorizontalZoom, -PaintPosVector.Y + 30))),
				FText::FromString(FString::FromInt(GridPoint.Subdivision)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
				ESlateDrawEffect::None,
				FLinearColor::White
			);


			LineColor = FLinearColor::Gray;
			break;

		case EGridPointType::Subdivision:
			//draw subdivision number
			FSlateDrawElement::MakeText(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * HorizontalZoom, -PaintPosVector.Y + 30))),
				FText::FromString(FString::FromInt(GridPoint.Subdivision)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
				ESlateDrawEffect::None,
				FLinearColor::White
			);

			LineColor = FLinearColor::Black;
			break;

		case EGridPointType::Beat:
			//draw beat number
			FSlateDrawElement::MakeText(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * HorizontalZoom, -PaintPosVector.Y + 18))),
				FText::FromString(FString::FromInt(GridPoint.Beat)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
				ESlateDrawEffect::None,
				FLinearColor::White
			);

			//draw subdivision number
			FSlateDrawElement::MakeText(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * HorizontalZoom, -PaintPosVector.Y + 30))),
				FText::FromString(FString::FromInt(GridPoint.Subdivision)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
				ESlateDrawEffect::None,
				FLinearColor::White
			);

			LineColor = FLinearColor::Blue;
			break;


		}

		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId - 1,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * HorizontalZoom, 0))),
			vertLine,
			ESlateDrawEffect::None,
			LineColor,
			false,
			FMath::Max(5.0f * HorizontalZoom, 1.0f));

	}



	

	for (const auto& TempoInfoPoint : MidiSongMap->GetTempoMap().GetTempoPoints())
	{
		//draw a square every tempo point
		FSlateDrawElement::MakeBox(OutDrawElements,
			timelineLayerID,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(5.0f, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(TempoInfoPoint.StartTick) * HorizontalZoom, -PaintPosVector.Y + 32))),
			&gridBrush,
			ESlateDrawEffect::None,
			FLinearColor::Red
		);

	}
#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION >= 5)
	for (size_t i = 0; i < MidiSongMap->GetNumTimeSignatureChanges(); i++)
	{
		//draw a blue box at each time sig change
		auto TimeSignaturePoint = MidiSongMap->GetTimeSignaturePoint(i);

		FSlateDrawElement::MakeBox(OutDrawElements,
			timelineLayerID,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(5.0f, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(TimeSignaturePoint->StartTick) * HorizontalZoom, -PaintPosVector.Y + 42))),
			&gridBrush,
			ESlateDrawEffect::None,
			FLinearColor::White
		);

	}
#endif


	//snapped mouse cursor
	FSlateDrawElement::MakeLines(OutDrawElements,
		timelineLayerID++,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(ValueAtMouseCursorPostSnapping)* HorizontalZoom, 0))),
		vertLine,
		ESlateDrawEffect::None,
		FLinearColor(255, 0, 255),
		false,
		FMath::Max(2.0f * HorizontalZoom, 1.0f));


	//draw play cursor
	FSlateDrawElement::MakeLines(OutDrawElements,
		LayerId,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(CurrentTimelinePosition* HorizontalZoom * 1000, 0))),
		vertLine,
		ESlateDrawEffect::None,
		FLinearColor::Red,
		false,
		FMath::Max(5.0f * HorizontalZoom, 1.0f));


	return timelineLayerID;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE