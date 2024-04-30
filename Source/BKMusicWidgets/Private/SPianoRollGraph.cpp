// Fill out your copyright notice in the Description page of Project Settings.


#include "SPianoRollGraph.h"
#include "SlateOptMacros.h"
#include "Logging/StructuredLog.h"
#include <SMidiNoteContainer.h>
//#include <BKMusicWidgets.h>

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


// internal function that converts between the enum types
EMidiClockSubdivisionQuantization TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits inTimeSpan)
{
	switch (inTimeSpan)
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
	PluginDir = IPluginManager::Get().FindPlugin(TEXT("unDAW"))->GetBaseDir();
	
	bCanSupportFocus = true;
	//tick


	gridColor = InArgs._gridColor;
	accidentalGridColor = InArgs._accidentalGridColor;
	cNoteColor = InArgs._cNoteColor;
	noteColor = InArgs._noteColor;
	pixelsPerBeat = InArgs._pixelsPerBeat;


	ChildSlot
		[
			SAssignNew(RootConstraintCanvas, SConstraintCanvas)
		];

	CursorTest = FBKMusicWidgetsModule::GetMeasuredGlyphFromHex(0xF040);
	
	RecalculateSlotOffsets();

	for (int i = 0; i <= 127; i++)
	{
		if (i%12 == 0){
			colorsArray.Add(&cNoteColor);
		}else{
			colorsArray.Add(UEngravingSubsystem::IsNoteInCmajor(i) ? &gridColor : &accidentalGridColor);
		}
	}

	FString test = FString(UEngravingSubsystem::pitchNumToStringRepresentation(61));
	//UE_LOGFMT(LogTemp, Log, "consexpr test: {0}", test);
}
void SPianoRollGraph::RecalculateSlotOffsets()
{
	rowHeight = 200 * verticalZoom;

	vertLine.Empty();
	vertLine.Add(FVector2f(0.0f, 0.0f));
	vertLine.Add(FVector2f(0.0f, 128 * rowHeight));

}
void SPianoRollGraph::AddNote(FLinkedMidiEvents& inNote, int inTrackSlot, int inInternalMidiTrackID)

{
	MidiSongMap = HarmonixMidiFile->GetSongMaps();
	auto newSlot = FZoomablePanelSlotContainer(&inNote, inTrackSlot);
	newSlot.trackindexInHarmonixMidi = inInternalMidiTrackID;
	newSlot.time = MidiSongMap->TickToMs(inNote.StartEvent.GetTick());
	newSlot.duration = MidiSongMap->TickToMs(inNote.EndEvent.GetTick()) - newSlot.time ;


	//this is very bad code nuke it soon.
	// month later still not nuked... 
	int totalNotes = slotsContainer.Add(newSlot);
	newSlot.uniqueMapKey = totalNotes;

	float velocity = inNote.StartEvent.GetMsg().Data2;

	FString noteName = UEngravingSubsystem::pitchNumToStringRepresentation(inNote.StartEvent.GetMsg().Data1);
	TSharedPtr<SMidiNoteContainer> TempButton;
	FString tooltext = FString::Printf(TEXT("Time in miliseconds %f \n Pitch: %d \n Velocity: %f, Name: %s \n TrackID: %d"), newSlot.time, inNote.StartEvent.GetMsg().Data1, velocity, *noteName, inInternalMidiTrackID);
	

	RootConstraintCanvas->AddSlot()
		.Offset(TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateLambda([&, data = newSlot]()
			{
				FZoomablePanelSlotContainer slotInLambda = data;

				float Left = slotMap[data.uniqueMapKey].time * horizontalZoom;
				float top = slotMap[data.uniqueMapKey].pitch * rowHeight;

				return FMargin(Left, top);

			})))
		.AutoSize(true)
		.Alignment(FVector2D(0.0f, 0.0f))
		.ZOrder(inTrackSlot)
		.Expose(newSlot.slotPointer)
		[
			SAssignNew(TempButton, SMidiNoteContainer)

				.ToolTipText(FText::FromString(tooltext))
				.Color(TAttribute<FLinearColor>::Create(TAttribute<FLinearColor>::FGetter::CreateLambda([&, data = inTrackSlot, data2 = newSlot]() {
					//UE_LOG(LogTemp, Log, TEXT("The value is %f"), static_cast<float>(data2.MidiNoteData->StartEvent.GetMsg().GetStdData2()))
						return parentMidiEditor->GetTracksDisplayOptions(data).trackColor.CopyWithNewOpacity(static_cast<float>(data2.MidiNoteData->StartEvent.GetMsg().GetStdData2()) * (1.5f/127.0f));
					})))
					.Size(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateLambda([&, data = newSlot]() {

						return FVector2D(data.duration * horizontalZoom, rowHeight);
					})))
				.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda([&, data = inTrackSlot](){
					EVisibility viewState = EVisibility::Visible;
					viewState = parentMidiEditor->GetTracksDisplayOptions(data).isVisible ? EVisibility::Visible : EVisibility::Hidden;
					return viewState;
					
					})))
				.CornerRadius(FVector4(1.0f, 1.0f, 1.0f, 1.0f))
				.UIDinParent(totalNotes)

		];

	TempButton.Get()->SetParentSharedPtr(selfSharedPtr);
	slotMap.Add(totalNotes, newSlot);

}
void SPianoRollGraph::InitFromLinkedMidiData(TMap<int, TArray<FLinkedMidiEvents*>> inLinkedNoteDataMap)
{
	UE_LOG(LogTemp, Log, TEXT("Init from linked midi data"))
	MidiSongMap = HarmonixMidiFile->GetSongMaps();
		LinkedNoteDataMap.Append(inLinkedNoteDataMap);
}
void SPianoRollGraph::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	//this is the zoom smoothing function, it is not stable.
	if (horizontalZoom != hZoomTarget)
	{
		//here we keep our zoom position close to where the mouse was, I need to also add interpolation towards the center
		//and make this 2d instead of 1d
		
		float preZoomTimeAtMouse = localMousePosition.X / horizontalZoom;

		horizontalZoom = FMath::Lerp<float>(horizontalZoom, hZoomTarget, 0.2f);

		float postZoomTimeAtMouse = localMousePosition.X / horizontalZoom;
		float timeDelta = postZoomTimeAtMouse - preZoomTimeAtMouse;
		positionOffset.X += timeDelta * horizontalZoom;
		positionOffset.X = FMath::Min(positionOffset.X, 0.0f);

	}

	// here we perform whatever tick logics related to the play head cursor
	if (WorldContextObject != nullptr)
	{
	
		double AudioTime = UGameplayStatics::GetAudioTimeSeconds(WorldContextObject);
		switch (TransportPlaystate)
		{
			
		case Preparing:
			break;
		case ReadyToPlay:
			break;
		case Playing:
		//to track the playhead accurately we want to calculate the delta audio time, but due to some quirks with unreal
		//we can't rely on the audio time measurement if the user interacts with the GUI, so if we're playing and suddenly the audio delta is 0,
		//such as what happens when the user pans the graph, then we fall back on the normal delta time.
			//if (AudioTime - LastMeasuredAudioTime > 0)
			//{
			//	parentMidiEditor->AddDeltaToTimeLine(AudioTime - LastMeasuredAudioTime);
			//}
			//else {
			//	parentMidiEditor->AddDeltaToTimeLine(InDeltaTime);
			//}
			//UE_LOG(LogTemp,Log, TEXT("Audio Time: %f, Previous Measurement: %f, Audio Delta Time: %f"), AudioTime, LastMeasuredAudioTime, AudioTime - LastMeasuredAudioTime)
			break;
		case Seeking:
			break;
		case Paused:
			break;
		default:
			break;
		}
		LastMeasuredAudioTime = AudioTime;
	}

	
	if (parentMidiEditor->getFollowCursor())
	{
			float currentTickTimeLinePosition = parentMidiEditor->currentTimelineCursorPosition;
			positionOffset.X -= (currentTickTimeLinePosition - LastTickTimelinePosition) * horizontalZoom * 1000.0f;
			LastTickTimelinePosition = currentTickTimeLinePosition;
	}

	positionOffset.X = FMath::Min(positionOffset.X, 0.0f);




	//update cursor and the such 
	if (!receivingDragUpdates)
	{
		int tickAtMouse = MidiSongMap->MsToTick(localMousePosition.X / horizontalZoom);
		CurrentBeatAtMouseCursor = MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse).Beat;
		CurrentBarAtMouseCursor = MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse).Bar;
		//int toTickBar = MidiSongMap->GetBarMap().MusicTimestampBarBeatTickToTick(bar, beat, 0);
		//int PrevBeatTick = toTickBar + (MidiSongMap->GetTicksPerQuarterNote() * (beat - 1));//+ MidiSongMap->GetTicksPerQuarterNote() * MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), toTickBar);
		ValueAtMouseCursorPostSnapping = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse), TimeSpanToSubDiv(QuantizationGridUnit));
		if (QuantizationGridUnit == EMusicTimeSpanOffsetUnits::Ms) ValueAtMouseCursorPostSnapping = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse), EMidiClockSubdivisionQuantization::None);

	}


}
void SPianoRollGraph::SetInputMode(EPianoRollEditorMouseMode newMode)
{
	InputMode = newMode;
	switch (newMode) {
	case drawNotes:
		if (isCtrlPressed)
		{
			CursorTest = FBKMusicWidgetsModule::GetMeasuredGlyphFromHex(0xF014);
		}else{
			CursorTest = FBKMusicWidgetsModule::GetMeasuredGlyphFromHex(0xF040);
		}
		break;

	case notesSelect:
		CursorTest = FMeasuredGlyph{ TCHAR(0xF05B), 0.0f, 0.0f};
		break;


	case pan:
		CursorTest = FMeasuredGlyph{ TCHAR(0xF047), 0.0f, 0.0f };
		break;
	case seek:
		CursorTest = FMeasuredGlyph{ TCHAR(0xF028), 0.0f, 0.0f };
		break;
	default:
		CursorTest = FBKMusicWidgetsModule::GetMeasuredGlyphFromHex(0xF00E);
		break;
	}


}
void SPianoRollGraph::AddHorizontalX(float inputX)
{
	positionOffset.X += inputX;
	positionOffset = FVector2f::Min(positionOffset, FVector2f::ZeroVector);

	RecalcGrid();
}
void SPianoRollGraph::UpdateSlotsZOrder()
{
	for (auto slot = slotMap.CreateIterator(); slot; ++slot)
	{
		float slotZorder = parentMidiEditor->GetTracksDisplayOptions(slot.Value().trackID).isSelected ? 1.0f : 0.0f;
		
		slot.Value().slotPointer->SetZOrder(slotZorder);
	};
}

//to delete
void SPianoRollGraph::ResetCanvas()
{
	//RootConstraintCanvas->ClearChildren();
	//RootConstraintCanvas.Reset();
}

FReply SPianoRollGraph::NoteClickedDelegate(FZoomablePanelSlotContainer InStruct) 
{
	slotMap[InStruct.uniqueMapKey].time += 0.5;
	UE_LOGFMT(LogTemp, Log, "Note Clicked, here is the time! {0}", slotMap[InStruct.uniqueMapKey].time);
	return FReply::Handled();
}
FReply SPianoRollGraph::NotePressed(FZoomablePanelSlotContainer notedata)
{
	isNotePressed = true;
	selectedNoteMapIndex = notedata.uniqueMapKey;
	
	return FReply::Handled();
}
void SPianoRollGraph::NotePressedVoid(FZoomablePanelSlotContainer notedata)
{
	isNotePressed = true;
	selectedNoteMapIndex = notedata.uniqueMapKey;
}

void SPianoRollGraph::CacheDesiredSize(float InLayoutScaleMultiplier) //Super::CacheDesiredSize(InLayoutScaleMultiplier)
{
		RecalcGrid();
};

void SPianoRollGraph::RecalcGrid()
{
	
	//after much figuring out, this is the code that generates the grid based on quantization size
	visibleBeats.Empty();
	float LeftMostTick = MidiSongMap->MsToTick(-positionOffset.X / horizontalZoom);
	float RightMostTick = MidiSongMap->MsToTick((GetCachedGeometry().GetLocalSize().X  -positionOffset.X) / horizontalZoom);
	//int GridBeat = MidiSongMap->GetBarMap().TickToMusicTimestamp(LeftMostTick).Beat;
	//int GridBars = MidiSongMap->GetBarMap().TickToMusicTimestamp(LeftMostTick).Bar;

	visibleBars.Empty();
	
	//populate subdiv array
	float subDivTick = LeftMostTick;
	while(subDivTick <= RightMostTick)
	{
		visibleBeats.Add(MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(subDivTick), TimeSpanToSubDiv(QuantizationGridUnit)));
		//visibleBars.Add(MidiSongMap->GetBarMap().MusicTimestampBarToTick(bars));
		subDivTick += MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), subDivTick);
			
	}


	//populate bar array 
	// can probably just leave it as time, rather than calculate back and forth...
	float barTick = LeftMostTick;
	while(barTick <= RightMostTick)
	{
		//MidiSongMap->GetBarMap()
		visibleBars.Add(MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(barTick), TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars)));
		//visibleBars.Add(MidiSongMap->GetBarMap().MusicTimestampBarToTick(bars));
		barTick += MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars), barTick);
			
	}


	//int bars = 0;
	//MidiSongMap->bars
	//while (bars <= MidiSongMap->GetSongLengthBars())
	//{
	//	bars++;
	//}

	//drawLength = -1 * positionOffset.X + GetCachedGeometry().GetAbsoluteSize().X;

	
	gridLine.Empty(2);
	//horizontal line
	gridLine.Add(FVector2f(-1 * positionOffset.X, 0.0f));
	gridLine.Add(FVector2f(drawLength, 0.0f));

	vertLine.Empty(2);
	//vertical line
	vertLine.Add(FVector2f(0.0f, 0.0f));
	vertLine.Add(FVector2f(0.0f, 127 * rowHeight));
}


FReply SPianoRollGraph::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//UE_LOG(LogTemp, Log, TEXT("Is this locking?"))
	
	if (HasKeyboardFocus())
	{
		
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !bLMBdown)
		{
			bLMBdown = true;
			return FReply::Handled();
			//return OnMouseMove(MyGeometry, MouseEvent);
			//return FReply::Unhandled().CaptureMouse(AsShared());
		}
	}


	return FReply::Unhandled();
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
		

		parentMidiEditor->UpdateTemporalZoom(hZoomTarget, WidgetSpaceCursorPos);

		absMousePosition = InMouseEvent.GetScreenSpacePosition();
		return FReply::Handled();
		
	}
	else if (isShiftPressed)
	{
		if (InMouseEvent.GetWheelDelta() >= 0.1)
		{
			verticalZoom *= 1.1f;
		}
		else {
			verticalZoom *= 0.9f;
		}
		verticalZoom = FMath::Clamp(verticalZoom, 0.01, 2.0);
		RecalculateSlotOffsets();
		return FReply::Handled();
	}
	


	return FReply::Unhandled();
}
FReply SPianoRollGraph::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const bool bIsRightMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
	const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
	const bool bIsMiddleMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton);
	const FModifierKeysState ModifierKeysState = FSlateApplication::Get().GetModifierKeys();


		auto abs = MouseEvent.GetScreenSpacePosition();
		localMousePosition = GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - positionOffset;
		hoveredPitch = 127 -FMath::Floor(localMousePosition.Y / rowHeight);

		if (bLMBdown) {
	
			switch (parentMidiEditor->getCurrentInputMode())
			{
			case seek:

				parentMidiEditor->SetCurrentPosition(MidiSongMap->TickToMs(ValueAtMouseCursorPostSnapping) / 1000.0f);
				break;

			case notesSelect:
		
				positionOffset.Y += MouseEvent.GetCursorDelta().Y;
				parentMidiEditor->AddHorizontalOffset(MouseEvent.GetCursorDelta().X);
			default:

				break;
			}

			return FReply::Handled().CaptureMouse(AsShared());
		}

	return FReply::Unhandled();
}




FReply SPianoRollGraph::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	isCtrlPressed = InKeyEvent.IsControlDown();
	isShiftPressed = InKeyEvent.IsShiftDown();
	if(KeyMappings != nullptr)
	{

		auto* newInput = KeyMappings->KeyMap.Find(InKeyEvent.GetKey());
		if (newInput != nullptr)
		{

			switch (newInput->GetValue())
			{
			case DrawModeSwitch:
				InputMode = drawNotes;
				break;

			case SeekMode:
				InputMode = seek;
				break;

			case SelectMode:
				InputMode = notesSelect;
				break;
			default:
				break;
			}

		}
	}

	parentMidiEditor->SetInputMode(InputMode);

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
	
	// bad stupid code, this should happen in the tick event
	auto PaintPosVector = positionOffset;

	
	auto OffsetGeometryChild = AllottedGeometry.MakeChild(AllottedGeometry.GetLocalSize(), FSlateLayoutTransform(1.0f,(FVector2d)PaintPosVector));
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
				!someTrackIsSelected || availableSamplesMap.Find(i) ? *colorsArray[i] : colorsArray[i]->operator+(FLinearColor(0.01f, 0.00f, 0.00f, 0.01f))
			);

		}

		
		int vertLineIndices = MyCullingRect.Right / pixelsPerBeat;
		int firstGridLine = MyCullingRect.Left / pixelsPerBeat;

		for (int i = firstGridLine; i <= vertLineIndices; i++)
		{
			FSlateDrawElement::MakeLines(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(firstGridLine + horizontalZoom * pixelsPerBeat * 1000 * i, 0))),
				vertLine,
				ESlateDrawEffect::None,
				FLinearColor::Blue,
				false,
				5.0f * horizontalZoom);
		}
	};

	//bar and beatlines, calculated earlier, not on the paint event
	for (auto& beat : visibleBeats)
	{
		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(beat) * horizontalZoom, 0))),
			vertLine,
			ESlateDrawEffect::None,
			FLinearColor::Gray.CopyWithNewOpacity(0.5f * horizontalZoom),
			false,
			FMath::Max(5.0f * horizontalZoom, 1.0f));

	}

	for (auto& bar : visibleBars)
	{
		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId,
			OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(bar) * horizontalZoom, 0))),
			vertLine,
			ESlateDrawEffect::None,
			FLinearColor::Blue.CopyWithNewOpacity(0.3f),
			false,
			FMath::Max(15.0f * horizontalZoom, 1.0f));
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



	// draw the child canvas, (that is all the actual notes) 
	int postCanvasLayerID = RootConstraintCanvas.Get()->Paint(Args, OffsetGeometryChild, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	//if in note draw mode, draw note overlay 
	
	if (parentMidiEditor.IsValid() && someTrackIsSelected && parentMidiEditor->getCurrentInputMode() == EPianoRollEditorMouseMode::drawNotes)
	{
		FLinearColor trackColor = parentMidiEditor->GetTracksDisplayOptions(selectedTrackIndex).trackColor;
		FLinearColor offTracksColor = trackColor.Desaturate(0.5);
		FLinearColor colorNegative = FLinearColor::White.operator-(trackColor);



		for (int i = 0; i <= 127; i++)
		{
			bool isSelectedNote = i == hoveredPitch;
			float opacity = (float)0.7f * (127.0f - FMath::Abs(i - hoveredPitch) * 12) / 127.0f;

			FSlateDrawElement::MakeBox(OutDrawElements,
				postCanvasLayerID++,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(-PaintPosVector.X, rowHeight * (127 - i)))),
				&gridBrush,
				ESlateDrawEffect::None,
				isSelectedNote ? trackColor.CopyWithNewOpacity(1.0f) :
				offTracksColor.CopyWithNewOpacity(opacity)
			);

			FSlateDrawElement::MakeText(OutDrawElements,
				postCanvasLayerID++,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(150, rowHeight), FSlateLayoutTransform(1.0f, FVector2D((double)-PaintPosVector.X, rowHeight * (127 - i)))),
				FText::FromString(FString::Printf(TEXT("%d"), i)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 9),
				ESlateDrawEffect::None,
				isSelectedNote ? colorNegative.CopyWithNewOpacity(1.0f) : colorNegative.CopyWithNewOpacity(opacity)
			);
		}
	}

	//here we're gonna run some tests, so maybe some of this code will be useful to debug later, surrounding it with the if def
	// allows us to completly remove this section when compiling, the actual define is in the header file, can easily be commented out.


	//mouse crosshairs
	FLinearColor trackColor = selectedTrackIndex >= 0 ? parentMidiEditor->GetTracksDisplayOptions(selectedTrackIndex).trackColor : FLinearColor::White;

	////ugly code, can't be in the paint event, needs to be in the mouse move or tick methods. 
	//int tickAtMouse = MidiSongMap->MsToTick(localMousePosition.X / horizontalZoom);
	//int beat = MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse).Beat;
	//int bar = MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse).Bar;
	int toTickBar = MidiSongMap->GetBarMap().MusicTimestampBarBeatTickToTick(CurrentBarAtMouseCursor, CurrentBeatAtMouseCursor, 0);
	int PrevBeatTick = toTickBar + (MidiSongMap->GetTicksPerQuarterNote() * (CurrentBeatAtMouseCursor - 1));//+ MidiSongMap->GetTicksPerQuarterNote() * MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), toTickBar);
	//auto PrevSubDivTick = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse), TimeSpanToSubDiv(QuantizationGridUnit));
	//if(QuantizationGridUnit == EMusicTimeSpanOffsetUnits::Ms) PrevSubDivTick = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse), EMidiClockSubdivisionQuantization::None);

#ifdef PIANO_ROLL_DEBUG

	FSlateDrawElement::MakeText(OutDrawElements,
	postCanvasLayerID++,
	AllottedGeometry.ToPaintGeometry(),
	FText::FromString(FString::Printf(TEXT("My Culling Rect Left, Right: %f, %f \n abs to local %s \n local size %s \n mouse %s \n zoom H:%f V:%f \n Position Offset %s \n num bars %d \n tick at local 0 mouse %f \n bar %d beat %d  "),

	OffsetGeometryChild.GetLayoutBoundingRect().Left, OffsetGeometryChild.GetLayoutBoundingRect().Right,
	*OffsetGeometryChild.AbsoluteToLocal(FVector2d::Zero()).ToString(),
	*OffsetGeometryChild.GetLocalSize().ToString(),
	*localMousePosition.ToString(),
	horizontalZoom, verticalZoom,
	*positionOffset.ToString(),
	visibleBars.Num(),
	MidiSongMap->MsToTick(-positionOffset.X / horizontalZoom),
		CurrentBarAtMouseCursor, CurrentBeatAtMouseCursor)),
	FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12),
	ESlateDrawEffect::None,
	FLinearColor::White);

	//cursor tests
	FSlateDrawElement::MakeText(OutDrawElements,
		postCanvasLayerID++,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, localMousePosition.operator+(FVector2D(50,0)))),
		FText::FromString(FString::Printf(TEXT("mouse %s \n time at mouse %f \n cursor position %f \n Tick at mouse %f"),	* localMousePosition.ToString()
			, localMousePosition.X / horizontalZoom, CurrentTimelinePosition, MidiSongMap->MsToTick(localMousePosition.X / horizontalZoom))),
		FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 8),
		ESlateDrawEffect::None,
		FLinearColor::White);

	if(false) {

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

	for (auto& track : LinkedNoteDataMap)
	{
		
		
		for (auto& note : track.Value)
		{
			double duration = note->EndEvent.GetTick() - note->StartEvent.GetTick();
			
			FSlateDrawElement::MakeBox(OutDrawElements,
				postCanvasLayerID++,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(duration * horizontalZoom, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(note->StartEvent.GetTick() * horizontalZoom, rowHeight * (127 - note->StartEvent.GetMsg().Data1)))),
				&gridBrush,
				ESlateDrawEffect::None,
				parentMidiEditor->GetTracksDisplayOptions(track.Key).trackColor.CopyWithNewOpacity(note->StartEvent.GetMsg().Data2 / 127.0f)
			);
		}
	}

	//snapped mouse cursor
	FSlateDrawElement::MakeLines(OutDrawElements,
		LayerId,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(MaxWidth, rowHeight), FSlateLayoutTransform(1.0f, FVector2D(MidiSongMap->TickToMs(ValueAtMouseCursorPostSnapping)* horizontalZoom, 0))),
		vertLine,
		ESlateDrawEffect::None,
		FLinearColor(255, 0, 255),
		false,
		FMath::Max(2.0f * horizontalZoom, 1.0f));

	FSlateDrawElement::MakeText(OutDrawElements,
		postCanvasLayerID++,
		OffsetGeometryChild.ToPaintGeometry(FVector2D(1.0f, 1.0f), FSlateLayoutTransform(1.0f, localMousePosition-FVector2D(0.0f, CursorTest.measuredY))),
		&CursorTest.glyph,
		FSlateFontInfo(PluginDir / TEXT("Resources/UtilityIconsFonts/icons.ttf"), 24),
		ESlateDrawEffect::None,
		trackColor.CopyWithNewOpacity(1.0f));
	

	return postCanvasLayerID;
}
void SPianoRollGraph::DragNote(const FPointerEvent& MouseEvent)
{

	auto abs = MouseEvent.GetScreenSpacePosition();
	localMousePosition = GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - positionOffset;
	receivingDragUpdates = true;
	
	//update cursor and the such 
	int tickAtMouse = MidiSongMap->MsToTick(localMousePosition.X / horizontalZoom);
	int tickAtNoteStart = slotMap[selectedNoteMapIndex].MidiNoteData->StartEvent.GetTick();
	//int delta = tickAtNoteStart - tickAtMouse;
	
	CurrentBeatAtMouseCursor = MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse).Beat;
	CurrentBarAtMouseCursor = MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse).Bar;
	int toTickBar = MidiSongMap->GetBarMap().MusicTimestampBarBeatTickToTick(CurrentBarAtMouseCursor, CurrentBeatAtMouseCursor, 0);
	int PrevBeatTick = toTickBar + (MidiSongMap->GetTicksPerQuarterNote() * (CurrentBeatAtMouseCursor - 1));//+ MidiSongMap->GetTicksPerQuarterNote() * MidiSongMap->SubdivisionToMidiTicks(TimeSpanToSubDiv(QuantizationGridUnit), toTickBar);
	ValueAtMouseCursorPostSnapping = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse), TimeSpanToSubDiv(QuantizationGridUnit));
	//if (QuantizationGridUnit == EMusicTimeSpanOffsetUnits::Ms) ValueAtMouseCursorPostSnapping = MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(tickAtMouse), EMidiClockSubdivisionQuantization::None);

	hoveredPitch = 127 - FMath::Floor(localMousePosition.Y / rowHeight);
	slotMap[selectedNoteMapIndex].pitch = FMath::Floor(localMousePosition.Y / rowHeight);
	slotMap[selectedNoteMapIndex].UpdateNotePitch(static_cast<uint8>(hoveredPitch));
	UE_LOG(LogTemp, Log, TEXT("Time before we destroy it %f, after %d"), slotMap[selectedNoteMapIndex].time, ValueAtMouseCursorPostSnapping)
	slotMap[selectedNoteMapIndex].UpdateNoteStartTime(MidiSongMap->TickToMs(ValueAtMouseCursorPostSnapping), ValueAtMouseCursorPostSnapping);
	

	
	

	
}

void SPianoRollGraph::StopDraggingNote()
{
	receivingDragUpdates = false;
	auto& data = slotMap[selectedNoteMapIndex];
	const int& trackID = slotMap[selectedNoteMapIndex].trackindexInHarmonixMidi;

	//stop and destroy the audio component if it exists and playing
	if (PerformanceComponent != nullptr && PerformanceComponent->IsValidLowLevel()) {
		PerformanceComponent->Stop();
		//destroy it
		PerformanceComponent->DestroyComponent();
	}

	


		//add new events
	auto& rawEvents = HarmonixMidiFile->GetTrack(trackID)->GetRawEvents();
	//rawEvents.
	auto iterator = HarmonixMidiFile->GetTrack(trackID)->GetRawEvents().CreateConstIterator();
		
		//HarmonixMidiFile->GetTrack(trackID)->AddEvent(data.MidiNoteData->StartEvent);
		//HarmonixMidiFile->GetTrack(trackID)->AddEvent(data.MidiNoteData->EndEvent);
		//HarmonixMidiFile->GetTrack(trackID)->Sort();
		//HarmonixMidiFile->GetTrack(trackID)->
	

		//HarmonixMidiFile->
		//remove existing note using event index
		//HarmonixMidiFile->GetTrack(trackID)->GetEvents().(data.MidiNoteData->EndIndex);
		//HarmonixMidiFile->GetTrack(trackID)->Sort();
		HarmonixMidiFile->GetTrack(trackID)->GetRawEvents().Remove(data.MidiNoteData->StartEvent);
		HarmonixMidiFile->GetTrack(trackID)->GetRawEvents().Remove(data.MidiNoteData->EndEvent);
		//HarmonixMidiFile->GetTrack(trackID)->Sort();
	
		//call all the sorting functions I could find
		
		HarmonixMidiFile->SortAllTracks();
		//HarmonixMidiFile->TracksChanged();
		HarmonixMidiFile->MarkPackageDirty();

		//NeedsRinitDelegate.Broadcast();

}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
