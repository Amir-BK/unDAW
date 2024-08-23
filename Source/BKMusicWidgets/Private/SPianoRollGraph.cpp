// Fill out your copyright notice in the Description page of Project Settings.

#include "SPianoRollGraph.h"
#include "SlateOptMacros.h"
#include "Logging/StructuredLog.h"
#include "ITimeSlider.h"
#include "Widgets/Input/SButton.h"
#include "Algo/BinarySearch.h"

//#include <BKMusicWidgets.h>

#define LOCTEXT_NAMESPACE "PianoRollGraph"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

DEFINE_LOG_CATEGORY(SPIANOROLLLOG);

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
	auto KeyMapsSoftPath = FSoftObjectPath("/unDAW/KeyboardMappings/MidiEditorKeyBindings.MidiEditorKeyBindings");
	KeyMappings = Cast<UBKEditorUtilsKeyboardMappings>(KeyMapsSoftPath.TryLoad());

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

	//FString test = FString(UEngravingSubsystem::pitchNumToStringRepresentation(61));

	//SetCurrentTimestamp(InArgs._CurrentTimestamp);
	//CurrentTimestamp.

	//Init();
	//UE_LOGFMT(LogTemp, Log, "consexpr test: {0}", test);
}
void SPianoRollGraph::RecalculateSlotOffsets()
{
	rowHeight = 200 * verticalZoom;

	vertLine.Empty();
	vertLine.Add(FVector2f(0.0f, 0.0f));
	vertLine.Add(FVector2f(0.0f, 128 * rowHeight));
}

//void SPianoRollGraph::Init()
//{
//	if(!SessionData) return;
//
//	OnSeekEvent.BindUObject(SessionData, &UDAWSequencerData::SendSeekCommand);
//
//
//	if (SessionData->HarmonixMidiFile)
//	{
//		SetCanTick(true);
//		//MidiSongMap = SessionData->HarmonixMidiFile->GetSongMaps();
//		LinkedNoteDataMap = &SessionData->LinkedNoteDataMap;
//
//		InputMode = EPianoRollEditorMouseMode::Panning;
//
//	}
//	else {
//
//		SetCanTick(false);
//		InputMode = EPianoRollEditorMouseMode::empty;
//	}
//
//}

void SPianoRollGraph::SetSessionData(UDAWSequencerData* inSessionData)
{
	UE_LOG(SPIANOROLLLOG, Log, TEXT("Setting Session Data!"));

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

	if (bFollowCursor)
	{
		//UE_LOG(LogTemp, Log, TEXT("Updating Timestamp! New Time Stamp bar %f new timeline position %f"), newTimestamp, CurrentTimelinePosition);

		float DesiredCursorPosition = (GetCachedGeometry().GetLocalSize().X - PianoTabWidth.Get()) * CursorFollowAnchorPosition - PositionOffset.X;
		float LocalSpacePlayBackPosition = CurrentTimeMiliSeconds * horizontalZoom; //+ positionOffset.X;

		PositionOffset.X = -LocalSpacePlayBackPosition;// + DesiredCursorPosition;
		//UE_LOG(SPIANOROLLLOG, Log, TEXT("Display is ahead of playback. Beginning of screen %f, LocalSpacePlayBackPosition %f"), DesiredCursorPosition, LocalSpacePlayBackPosition);

		//if (LocalSpacePlayBackPosition > DesiredCursorPosition)
		//{
		//	float DeltaPos = LocalSpacePlayBackPosition - DesiredCursorPosition;
		//	positionOffset.X = CurrentTimelinePosition;
		//}

		//check if we're ahead of the cursor and lerp to it

		float BeginningOfScreen = -PositionOffset.X;

		//UE_LOG(SPIANOROLLLOG, Log, TEXT("Display is ahead of playback. Beginning of screen %f, LocalSpacePlayBackPosition %f"), BeginningOfScreen, LocalSpacePlayBackPosition);
		//if (LocalSpacePlayBackPosition < BeginningOfScreen)
		//{
		//	float DeltaPos = LocalSpacePlayBackPosition - BeginningOfScreen;
		//	positionOffset.X = CurrentTimeMiliSeconds;

		//}
	}

	//this is the zoom smoothing function, it is not stable.
	if (horizontalZoom != hZoomTarget)
	{
		//here we keep our zoom position close to where the mouse was, I need to also add interpolation towards the center
		//and make this 2d instead of 1d

		float preZoomTimeAtMouse = localMousePosition.X / horizontalZoom;

		horizontalZoom = FMath::Lerp<float>(horizontalZoom, hZoomTarget, 0.2f);

		float postZoomTimeAtMouse = localMousePosition.X / horizontalZoom;
		float timeDelta = postZoomTimeAtMouse - preZoomTimeAtMouse;
		PositionOffset.X += timeDelta * horizontalZoom;
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
		tickAtMouse = MidiSongMap->MsToTick((localMousePosition.X - PianoTabWidth.Get()) / horizontalZoom);
		CurrentBeatAtMouseCursor = MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse).Beat;
		CurrentBarAtMouseCursor = MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse).Bar;
		//int toTickBar = MidiSongMap->GetBarMap().MusicTimestampBarBeatTickToTick(bar, beat, 0);
		//int PrevBeatTick = toTickBar + (MidiSongMap->GetTicksPerQuarterNote() * (beat - 1));//+ MidiSongMap->GetTicksPerQuarterNote() * MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), toTickBar);
		ValueAtMouseCursorPostSnapping = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse), TimeSpanToSubDiv(QuantizationGridUnit));
		if (QuantizationGridUnit == EMusicTimeSpanOffsetUnits::Ms) ValueAtMouseCursorPostSnapping = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse), EMidiClockSubdivisionQuantization::None);
	}

	//we need to update hovered pitch every tick
	HoveredPitch = 127 - FMath::Floor(localMousePosition.Y / rowHeight);

	if (InputMode == EPianoRollEditorMouseMode::drawNotes && SessionData->SelectedTrackIndex != INDEX_NONE)
	{
		if (!isCtrlPressed && HoveredPitch != LastDrawnNotePitch || LastDrawnNoteStartTick != ValueAtMouseCursorPostSnapping)
		{
			//calculate end tick, for now just add the quantization value to the start stick
			auto NumTicksPerSubdivision = MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), ValueAtMouseCursorPostSnapping);

			FLinkedMidiEvents newNote = FLinkedMidiEvents();
			newNote.NoteVelocity = NewNoteVelocity;
			auto TrackMetadata = SessionData->GetTracksDisplayOptions(SessionData->SelectedTrackIndex);
			newNote.pitch = HoveredPitch;
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

		//if in note draw mode, add note to pending notes map
		//TimeAtMouse
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
	PositionOffset = FVector2f::Min(PositionOffset, FVector2f::ZeroVector);

	RecalcGrid();
}

void SPianoRollGraph::CacheDesiredSize(float InLayoutScaleMultiplier) //Super::CacheDesiredSize(InLayoutScaleMultiplier)
{
	RecalcGrid();
};

void SPianoRollGraph::RecalcGrid()
{
	if (SessionData == nullptr) return;

	auto* MidiSongMap = MidiFile->GetSongMaps();
	//after much figuring out, this is the code that generates the grid based on quantization size
	VisibleBeats.Empty();
	float LeftMostTick = MidiSongMap->MsToTick(-PositionOffset.X / horizontalZoom);
	float RightMostTick = MidiSongMap->MsToTick((GetCachedGeometry().GetLocalSize().X - PositionOffset.X) / horizontalZoom);
	//int GridBeat = MidiSongMap->GetBarMap().TickToMusicTimestamp(LeftMostTick).Beat;
	//int GridBars = MidiSongMap->GetBarMap().TickToMusicTimestamp(LeftMostTick).Bar;

	VisibleBars.Empty();
	VisibleSubdivisions.Empty();
	//GridPointMap.Empty();
	//populate bar array
// can probably just leave it as time, rather than calculate back and forth...
	float barTick = LeftMostTick;
	while (!MidiSongMap->GetBarMap().IsEmpty() && barTick <= RightMostTick)
	{
		//MidiSongMap->GetBarMap()
		auto BarTick = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(barTick), TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars));
		VisibleBars.Add(BarTick);
		//visibleBars.Add(MidiSongMap->GetBarMap().MusicTimestampBarToTick(bars));
		FMusicalGridPoint BarGridPoint = { };
		//GridPointMap.Add(BarTick, BarGridPoint);

		barTick += MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars), barTick);
	}


	//populate beats array
	float subDivTick = LeftMostTick;
	while (!MidiSongMap->GetBarMap().IsEmpty() && subDivTick <= RightMostTick)
	{
		VisibleBeats.Add(MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(subDivTick), TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Beats)));
		//visibleBars.Add(MidiSongMap->GetBarMap().MusicTimestampBarToTick(bars));

	
		subDivTick += MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Beats), subDivTick);
	}

	//print quantization grid unit 
	

	// if we're not in beats mode, we need to calculate the subdivisions
	subDivTick = LeftMostTick;
	if(QuantizationGridUnit != EMusicTimeSpanOffsetUnits::Beats)
	{
		int SubDivCount = 1;
		while (!MidiSongMap->GetBarMap().IsEmpty() && subDivTick <= RightMostTick)
		{
		//VisibleSubdivisions.Add(MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(subDivTick), TimeSpanToSubDiv(QuantizationGridUnit)));
		if (GridPointMap.Contains(subDivTick))
		{
			SubDivCount = 1;
		}
		else {
			//GridPointMap.Add(subDivTick, { EGridPointType::Subdivision, ++SubDivCount });
		}
		
		subDivTick += MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), subDivTick);
		

		}
	}




	//cull notes
	CulledNotesArray.Empty();



	if (!LinkedNoteDataMap) return;

	for (auto& track : *LinkedNoteDataMap)
	{
		for (auto& note : track.Value.LinkedNotes)
		{
			bool NoteInRightBound = note.StartTick < RightMostTick;

			//as tracks are sorted we can assume that if we reached the right bound of the screen, we can break the loop, no, we actually CAN'T
			// cause tracks
			//if (!NoteInRightBound) {
			//	//This is the bad note? print its values
			//	UE_LOG(SPIANOROLLLOG, Log, TEXT("Bad Note! StartTick %d, EndTick %d, RightMostTick %f"), note.StartTick, note.EndTick, RightMostTick);

			//	}
			if (note.EndTick >= LeftMostTick)
			{
				//if notes are too small, don't draw
				//if(note->Duration * horizontalZoom >= 1.0f)

				CulledNotesArray.Add(&note);
				//note->cornerRadius = FMath::Clamp(note->Duration * horizontalZoom, 1.0f, 10.0f);
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
	vertLine.Add(FVector2f(0.0f, 127 * rowHeight));
}

void SPianoRollGraph::RecalcSubdivisions()
{
	if (SessionData == nullptr) return;

	const auto& MidiSongMap = SessionData->HarmonixMidiFile->GetSongMaps();
	UE_LOG(SPIANOROLLLOG, Log, TEXT("Quantization Grid Unit: %s"), *UEnum::GetValueAsString(QuantizationGridUnit));
	GridPointMap.Empty();

	// this time we're populating the entire grid point map
	const float FirstTickOfFirstBar = MidiSongMap->GetBarMap().BarBeatTickIncludingCountInToTick(1,1, 0);
	const float LastTickOfLastBar = SessionData->HarmonixMidiFile->GetLastEventTick();

	int32 BarCount = 1;
	float BarTick = FirstTickOfFirstBar;
	while (BarTick <= LastTickOfLastBar)
	{
		//VisibleBars.Add(MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(BarTick), TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars)));
		GridPointMap.Add(BarTick, { EGridPointType::Bar, BarCount++, 1, 1 });
		BarTick += MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars), BarTick);
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

		BeatTick += MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Beats), BeatTick);
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

		subDivTick += MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), subDivTick);
	}


	



}

TSharedPtr<SWrapBox> SPianoRollGraph::GetQuantizationButtons()
{
	if (QuantizationButtons.IsValid()) return QuantizationButtons;

	SAssignNew(QuantizationButtons, SWrapBox)
		//.PreferredSize(FVector2D(500.0f, 100.0f))
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Ms"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::Ms)
		]
		+ SWrapBox::Slot()
		[
			SNew(SButton)
				.Text(FText::FromString("Bars"))
				.OnClicked(this, &SPianoRollGraph::OnQuantizationButtonClicked, EMusicTimeSpanOffsetUnits::Bars)
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
	newNote.pitch = mousePosition.Y;
	newNote.TrackId = 1;
	newNote.ChannelId = 1;
	newNote.CalculateDuration(MidiSongMap);

	return newNote;
}

FReply SPianoRollGraph::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//UE_LOG(LogTemp, Log, TEXT("Is this locking?"))

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

	}

	//if in note draw mode, add note to pending notes map
	//TimeAtMouse
	//SessionData->PendingLinkedMidiNotesMap.Add(1, CreateNoteAtMousePosition(localMousePosition, MidiSongMap, QuantizationGridUnit));
	//UE_LOG(LogTemp, Log, TEXT("Mouse Down! should have tried to create note!"));

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
		//const int32 ZoomLevelDelta = FMath::FloorToInt(InMouseEvent.GetWheelDelta());

		//parentMidiEditor->UpdateTemporalZoom(hZoomTarget, WidgetSpaceCursorPos);

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
	HoveredPitch = 127 - FMath::Floor(localMousePosition.Y / rowHeight);

	if (MidiFile == nullptr) return FReply::Unhandled();

	auto* MidiSongMap = MidiFile->GetSongMaps();

	tickAtMouse = MidiSongMap->MsToTick(localMousePosition.X / horizontalZoom);
	//hoveredNotePitch = -1;

	SelectedNote = nullptr;
	CulledNotesArray.FindByPredicate([&](FLinkedMidiEvents* note) {
		if (tickAtMouse >= note->StartTick && tickAtMouse <= note->EndTick)
		{
			if (note->pitch == HoveredPitch)
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
			if (note.pitch == HoveredPitch)
			{
				SelectedNote = &note;
				return true;
			}
		}
		;
		return false;
		});

	if (bLMBdown) {
		switch (InputMode)
		{
		case EPianoRollEditorMouseMode::seek:
			OnSeekEvent.ExecuteIfBound(MidiSongMap->TickToMs(ValueAtMouseCursorPostSnapping) / 1000.0f);
			//UE_LOG(LogTemp, Log, TEXT("Is Delegate bound?? %s"), OnSeekEvent.IsBound() ? TEXT("Yes") : TEXT("No"));

			//UE_LOG(LogTemp, Log, TEXT("Seeking to: %f"), MidiSongMap->TickToMs(ValueAtMouseCursorPostSnapping) / 1000.0f);
			//parentMidiEditor->SetCurrentPosition(MidiSongMap->TickToMs(ValueAtMouseCursorPostSnapping) / 1000.0f);
			break;

		case EPianoRollEditorMouseMode::Panning:

			PositionOffset.Y += MouseEvent.GetCursorDelta().Y;
			AddHorizontalX(MouseEvent.GetCursorDelta().X);
		default:

			break;
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
	isCtrlPressed = InKeyEvent.IsControlDown();
	isShiftPressed = InKeyEvent.IsShiftDown();
	if (KeyMappings != nullptr)
	{
		auto* newInput = KeyMappings->KeyMap.Find(InKeyEvent.GetKey());
		if (newInput != nullptr)
		{
			switch (newInput->GetValue())
			{
			case DrawModeSwitch:
				InputMode = EPianoRollEditorMouseMode::drawNotes;
				break;

			case SeekMode:
				InputMode = EPianoRollEditorMouseMode::seek;
				break;

			case SelectMode:
				InputMode = EPianoRollEditorMouseMode::Panning;
				break;

			case IncreaseQuantizationSnap:
				QuantizationGridUnit = IncQuantizationSnap(QuantizationGridUnit);
				RecalcSubdivisions();
				break;

			case DecreaseQuantizationSnap:
				QuantizationGridUnit = DecQuantizationSnap(QuantizationGridUnit);
				RecalcSubdivisions();
				break;

			default:
				break;
			}
		}
	}

	SetInputMode(InputMode);

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

	auto MarginVector = FVector2f(PianoTabWidth.Get(), 0);
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
				OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(0, rowHeight * (127 - i)))),
				&gridBrush,
				ESlateDrawEffect::None,
				!someTrackIsSelected || AvailableSamplesMap.Find(i) ? *colorsArray[i] : colorsArray[i]->operator+(FLinearColor(0.01f, 0.00f, 0.00f, 0.01f))
			);
		}

		//
		//int vertLineIndices = MyCullingRect.Right / pixelsPerBeat;
		//int firstGridLine = MyCullingRect.Left / pixelsPerBeat;

		//for (int i = firstGridLine; i <= vertLineIndices; i++)
		//{
		//	FSlateDrawElement::MakeLines(OutDrawElements,
		//		LayerId,
		//		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(firstGridLine + horizontalZoom * pixelsPerBeat * 1000 * i, 0))),
		//		vertLine,
		//		ESlateDrawEffect::None,
		//		FLinearColor::Blue,
		//		false,
		//		5.0f * horizontalZoom);
		//}
	};

	if (SessionData == nullptr) return LayerId;

	auto* MidiSongMap = MidiFile->GetSongMaps();
#ifdef LEGACY_BARS_BEATS



	//bar and beatlines, calculated earlier, not on the paint event
	for (auto& beat : VisibleBeats)
	{
		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId +5,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(beat) * horizontalZoom, 0))),
			vertLine,
			ESlateDrawEffect::None,
			FLinearColor::Blue,
			false,
			FMath::Max(5.0f * horizontalZoom, 1.0f));

		//draw beat number
		FSlateDrawElement::MakeText(OutDrawElements,
			LayerId + 10,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(beat) * horizontalZoom, -PaintPosVector.Y + 14))),
			FText::FromString(FString::FromInt(MidiSongMap->GetBarMap().TickToMusicTimestamp(beat).Beat)),
			FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 9),
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}

	for (auto& bar : VisibleBars)
	{
		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId + 9,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(bar) * horizontalZoom, 0))),
			vertLine,
			ESlateDrawEffect::None,
			FLinearColor::Gray,
			false,
			FMath::Max(15.0f * horizontalZoom, 1.0f));

		//draw bar number

		FSlateDrawElement::MakeText(OutDrawElements,
			LayerId + 20,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(bar) * horizontalZoom, - PaintPosVector.Y))),
			FText::FromString(FString::FromInt(MidiSongMap->GetBarMap().TickToMusicTimestamp(bar).Bar)),
			FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 9),
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}
#endif // LEGACY_BARS_BEATS

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
						LayerId + 20,
						OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * horizontalZoom, -PaintPosVector.Y))),
						FText::FromString(FString::FromInt(GridPoint.Bar)),
						FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 14),
						ESlateDrawEffect::None,
						FLinearColor::White
					);

					//draw beat number
					FSlateDrawElement::MakeText(OutDrawElements,
						LayerId + 10,
						OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick)* horizontalZoom, -PaintPosVector.Y + 18))),
						FText::FromString(FString::FromInt(GridPoint.Beat)),
						FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
						ESlateDrawEffect::None,
						FLinearColor::White
					);

					//draw subdivision number
					FSlateDrawElement::MakeText(OutDrawElements,
						LayerId + 10,
						OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick)* horizontalZoom, -PaintPosVector.Y + 30))),
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
						LayerId + 10,
						OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * horizontalZoom, -PaintPosVector.Y + 30))),
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
						LayerId + 10,
						OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * horizontalZoom, -PaintPosVector.Y + 18))),
						FText::FromString(FString::FromInt(GridPoint.Beat)),
						FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
						ESlateDrawEffect::None,
						FLinearColor::White
					);

					//draw subdivision number
					FSlateDrawElement::MakeText(OutDrawElements,
						LayerId + 10,
						OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick) * horizontalZoom, -PaintPosVector.Y + 30))),
						FText::FromString(FString::FromInt(GridPoint.Subdivision)),
						FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
						ESlateDrawEffect::None,
						FLinearColor::White
					);

					LineColor = FLinearColor::Blue;
					break;


			}

			FSlateDrawElement::MakeLines(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(Tick)* horizontalZoom, 0))),
				vertLine,
				ESlateDrawEffect::None,
				LineColor,
				false,
				FMath::Max(5.0f * horizontalZoom, 1.0f));

	}

	for (const auto& TempoInfoPoint : MidiSongMap->GetTempoMap().GetTempoPoints())
	{
		//draw a square every tempo point
		FSlateDrawElement::MakeBox(OutDrawElements,
			LayerId + 10,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(5.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(TempoInfoPoint.StartTick) * horizontalZoom, -PaintPosVector.Y + 32))),
			&gridBrush,
			ESlateDrawEffect::None,
			FLinearColor::Red
		);

	}
	for (size_t i = 0; i < MidiSongMap->GetNumTimeSignatureChanges(); i++)
	{
		//draw a blue box at each time sig change
		auto TimeSignaturePoint = MidiSongMap->GetTimeSignaturePoint(i);

		FSlateDrawElement::MakeBox(OutDrawElements,
			LayerId + 10,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(5.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(TimeSignaturePoint->StartTick) * horizontalZoom, -PaintPosVector.Y + 42))),
			&gridBrush,
			ESlateDrawEffect::None,
			FLinearColor::White
		);

	}



	

	//draw play cursor
	FSlateDrawElement::MakeLines(OutDrawElements,
		LayerId,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(CurrentTimelinePosition * horizontalZoom * 1000, 0))),
		vertLine,
		ESlateDrawEffect::None,
		FLinearColor::Red,
		false,
		FMath::Max(5.0f * horizontalZoom, 1.0f));

	//mouse crosshairs
	FLinearColor trackColor = SessionData->SelectedTrackIndex != INDEX_NONE ? SessionData->GetTracksDisplayOptions(SessionData->SelectedTrackIndex).trackColor : FLinearColor::White;

	int toTickBar = MidiSongMap->GetBarMap().MusicTimestampBarBeatTickToTick(CurrentBarAtMouseCursor, CurrentBeatAtMouseCursor, 0);
	int PrevBeatTick = toTickBar + (MidiSongMap->GetTicksPerQuarterNote() * (CurrentBeatAtMouseCursor - 1));//+ MidiSongMap->GetTicksPerQuarterNote() * MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), toTickBar);

	//paint hovered note
	if (SelectedNote != nullptr)
	{
		//	FSlateDrawElement::bord

		// get selected note color based on metadata and produce negative color
		auto& SelectedNoteTrackColor = SessionData->GetTracksDisplayOptions(SelectedNote->TrackId).trackColor;
		auto colorNegative = FLinearColor::White.operator-(SelectedNoteTrackColor);

		auto& note = SelectedNote;
		FSlateDrawElement::MakeBox(OutDrawElements,
			LayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D((note->Duration + 20) * horizontalZoom, rowHeight + (50 * verticalZoom)), FSlateLayoutTransform(1.0f, FVector2D((note->StartTime - 5.0f) * horizontalZoom, (-25.0f * verticalZoom) + rowHeight * (127 - note->pitch)))),
			&gridBrush,
			ESlateDrawEffect::None,
			colorNegative.CopyWithNewOpacity(0.5f)
		);
	}

	for (auto& note : CulledNotesArray)
	{
		TArray<FSlateGradientStop> GradientStops = { FSlateGradientStop(FVector2D(0,0), SessionData->GetTracksDisplayOptions(note->TrackId).trackColor) };
		FSlateDrawElement::MakeGradient(OutDrawElements,
			LayerId + note->TrackId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(note->Duration * horizontalZoom, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(note->StartTime * horizontalZoom, rowHeight * (127 - note->pitch)))),
			GradientStops, EOrientation::Orient_Horizontal, ESlateDrawEffect::None,
			FVector4f::One() * 2.0f
		);
	}

	for (const auto& note : SessionData->PendingLinkedMidiNotesMap)
	{
		TArray<FSlateGradientStop> GradientStops = { FSlateGradientStop(FVector2D(0,0), SessionData->GetTracksDisplayOptions(note.TrackId).trackColor) };
		FSlateDrawElement::MakeGradient(OutDrawElements,
			LayerId + note.TrackId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(note.Duration * horizontalZoom, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(note.StartTime * horizontalZoom, rowHeight * (127 - note.pitch)))),
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
			OffsetGeometryChild.ToPaintGeometry(FVector2D(note->Duration * horizontalZoom, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(note->StartTime * horizontalZoom, rowHeight * (127 - note->pitch)))),
			&gridBrush,
			ESlateDrawEffect::None,
			trackColor.CopyWithNewOpacity(0.5f)
		);
	}

	//#define PIANO_ROLL_DEBUG
#ifdef PIANO_ROLL_DEBUG

	for (const auto& TempoEvent : TempoEvents)
	{
		//draw a line for every tempo event
		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(TempoEvent.GetTick()) * horizontalZoom, 0))),
			vertLine,
			ESlateDrawEffect::None,
			FLinearColor::Red,
			false,
			FMath::Max(2.0f * horizontalZoom, 1.0f));
	}

	//draw a line for every time signature event
	for (const auto& TimeSigEvent : TimeSignatureEvents)
	{
		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(TimeSigEvent.GetTick()) * horizontalZoom, 0))),
			vertLine,
			ESlateDrawEffect::None,
			FLinearColor::Green,
			false,
			FMath::Max(2.0f * horizontalZoom, 1.0f));
	}

	FSlateDrawElement::MakeText(OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		FText::FromString(FString::Printf(TEXT("My Culling Rect Left, Right: %f, %f \n abs to local %s \n local size %s \n mouse %s \n zoom H:%f V:%f \n Position Offset %s \n num notes %d \n tick at local 0 mouse %f \n bar %d beat %d \n Current Quantization Value %s "),

			MyCullingRect.Left, MyCullingRect.Right,
			*OffsetGeometryChild.AbsoluteToLocal(FVector2d::Zero()).ToString(),
			*OffsetGeometryChild.GetLocalSize().ToString(),
			*localMousePosition.ToString(),
			horizontalZoom, verticalZoom,
			*positionOffset.ToString(),
			CulledNotesArray.Num(),
			MidiSongMap->MsToTick(-positionOffset.X / horizontalZoom),
			CurrentBarAtMouseCursor, CurrentBeatAtMouseCursor,
			*UEnum::GetValueAsString(QuantizationGridUnit))),
		FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12),
		ESlateDrawEffect::None,
		FLinearColor::White);

	//cursor tests
	FSlateDrawElement::MakeText(OutDrawElements,
		LayerId,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, localMousePosition.operator+(FVector2D(50, 0)))),
		FText::FromString(FString::Printf(TEXT("mouse %s \n time at mouse %f \n cursor position %f \n Tick at mouse %f"), *localMousePosition.ToString()
			, localMousePosition.X / horizontalZoom, CurrentTimelinePosition, MidiSongMap->MsToTick(localMousePosition.X / horizontalZoom))),
		FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 8),
		ESlateDrawEffect::None,
		FLinearColor::White);

	FString noteString = SelectedNote != nullptr ? SelectedNote->GetFormmatedString() : "No Note Selected";

	//cursor tests
	FSlateDrawElement::MakeText(OutDrawElements,
		LayerId,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, localMousePosition.operator+(FVector2D(50, 25)))),
		FText::FromString(*noteString),
		FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 8),
		ESlateDrawEffect::None,
		FLinearColor::White);

	if (false) {
		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(toTickBar) * horizontalZoom, 0))),
			vertLine,
			ESlateDrawEffect::None,
			FLinearColor::Yellow,
			false,
			FMath::Max(2.0f * horizontalZoom, 1.0f));

		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(PrevBeatTick) * horizontalZoom, 0))),
			vertLine,
			ESlateDrawEffect::None,
			FLinearColor::Green,
			false,
			FMath::Max(2.0f * horizontalZoom, 1.0f));
	}

	FSlateDrawElement::MakeLines(OutDrawElements,
		LayerId,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(localMousePosition.X, 0))),
		vertLine,
		ESlateDrawEffect::None,
		trackColor.CopyWithNewOpacity(1.0f),
		false,
		FMath::Max(2.0f * horizontalZoom, 1.0f));

	FSlateDrawElement::MakeLines(OutDrawElements,
		LayerId,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(0, localMousePosition.Y))),
		gridLine,
		ESlateDrawEffect::None,
		trackColor.CopyWithNewOpacity(1.0f),
		false,
		FMath::Max(2.0f * horizontalZoom, 1.0f));

#endif

	//snapped mouse cursor
	FSlateDrawElement::MakeLines(OutDrawElements,
		PostNotesLayerID,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(ValueAtMouseCursorPostSnapping) * horizontalZoom, 0))),
		vertLine,
		ESlateDrawEffect::None,
		FLinearColor(255, 0, 255),
		false,
		FMath::Max(2.0f * horizontalZoom, 1.0f));

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
	for (int i = 0; i <= 127; i++)
	{
		bool isSelectedNote = i == HoveredPitch;
		//float opacity = (float)0.7f * (127.0f - FMath::Abs(i - hoveredPitch) * 12) / 127.0f;
		FLinearColor TracksColor = UEngravingSubsystem::IsNoteInCmajor(i) ? FLinearColor::White : FLinearColor::Black;

		FSlateDrawElement::MakeBox(OutDrawElements,
			PostNotesLayerID++,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MarginVector.X, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(-PaintPosVector.X, rowHeight * (127 - i)))),
			&gridBrush,
			ESlateDrawEffect::None,
			TracksColor
		);

		TArray<FVector2D> HorizontalLinePoints;
		HorizontalLinePoints.Add(FVector2D(0, rowHeight));
		HorizontalLinePoints.Add(FVector2D(MarginVector.X, rowHeight));
		//also draw black lines for the white keys

		FSlateDrawElement::MakeLines(OutDrawElements,
			PostNotesLayerID++,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(0.0f, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(-PaintPosVector.X, rowHeight * (127 - i)))),
			HorizontalLinePoints,
			ESlateDrawEffect::None,
			FLinearColor::Black,
			false,
			2.0f);
	}

	for (const auto& [MetadataIndex, NoteNumber] : SessionData->CurrentlyActiveNotes)
	{
		FSlateDrawElement::MakeBox(OutDrawElements,
			PostNotesLayerID++,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MarginVector.X, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(-PaintPosVector.X, rowHeight * (127 - NoteNumber)))),
			&gridBrush,
			ESlateDrawEffect::None,
			SessionData->GetTracksDisplayOptions(MetadataIndex).trackColor
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
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(-PaintPosVector.X, rowHeight * (127 - i)))),
				&gridBrush,
				ESlateDrawEffect::None,
				offTracksColor.CopyWithNewOpacity(opacity)
			);

			FSlateDrawElement::MakeText(OutDrawElements,
				PostNotesLayerID++,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(150, rowHeight), FSlateLayoutTransform(1.0f, FVector2D((double)-PaintPosVector.X, rowHeight * (127 - i)))),
				FText::FromString(FString::Printf(TEXT("%d"), i)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 9),
				ESlateDrawEffect::None,
				isSelectedNote ? colorNegative.CopyWithNewOpacity(1.0f) : colorNegative.CopyWithNewOpacity(opacity)
			);
		}
	}

	return PostNotesLayerID;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE