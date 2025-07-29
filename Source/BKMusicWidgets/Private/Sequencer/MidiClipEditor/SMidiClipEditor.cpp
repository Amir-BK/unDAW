// Fill out your copyright notice in the Description page of Project Settings.

#include "Sequencer/MidiClipEditor/SMidiClipEditor.h"
#include "SlateOptMacros.h"
#include "UnDAWStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMidiClipEditor::Construct(const FArguments& InArgs, UDAWSequencerData* InSequence)
{
	SMidiEditorPanelBase::Construct(InArgs._ParentArgs, InSequence);
	SequenceData = InSequence;
	MajorTabWidth = 0.0f;
	TimelineHeight = InArgs._TimelineHeight;
	bLockVerticalPan = false;

	//populate piano grid colors 
	auto GridColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.1f);
	auto AccidentalColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.1f);
	auto CNoteGridColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.1f);

	for (int i = 0; i < 128; i++)
	{
		if (i % 12 == 1 || i % 12 == 3 || i % 12 == 6 || i % 12 == 8 || i % 12 == 10)
		{
			PianoGridColors.Add(AccidentalColor);
		}
		else if (i % 12 == 0)
		{
			PianoGridColors.Add(CNoteGridColor);
		}
		else
		{
			PianoGridColors.Add(GridColor);
		}
	}
}

int32 SMidiClipEditor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    static const FSlateBrush* NoteBrush = FUndawStyle::Get().GetBrush("MidiNoteBrush");

    const FLinearColor& TrackColorRef = SequenceData->GetTrackMetadata(TrackIndex).TrackColor;

    const auto& PlayCursorTick = SequenceData->HarmonixMidiFile->GetSongMaps()->CalculateMidiTick(PlayCursor.Get(), EMidiClockSubdivisionQuantization::None) - GetStartOffset();
    const float CursorPixel = TickToPixel(PlayCursorTick);

    // Draw piano grid
    LayerId++;
    for (int i = 0; i < 128; i++)
    {
        const float Y = (127 - i) * RowHeight + Position.Get().Y; // Add vertical offset

        // Draw grid background - Fix deprecated API
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(
                FVector2f(AllottedGeometry.Size.X, RowHeight),
                FSlateLayoutTransform(FVector2f(0, Y))
            ),
            NoteBrush,
            ESlateDrawEffect::None,
            PianoGridColors[i]
        );

        // Draw note names for C notes
        if (i % 12 == 0)
        {
            const auto NoteName = FText::FromString(FString::Printf(TEXT("C (%d)"), (i / 12) - 2));
            FSlateDrawElement::MakeText(
                OutDrawElements,
                LayerId++,
                AllottedGeometry.ToPaintGeometry(
                    FVector2f(50.0f, RowHeight),
                    FSlateLayoutTransform(FVector2f(0, Y))
                ),
                NoteName,
                FAppStyle::GetFontStyle("NormalFont"),
                ESlateDrawEffect::None,
                FLinearColor::White
            );
        }
    }

    // Draw MIDI notes
    if (Clip != nullptr)
    {
        LayerId++;
        for (const auto& Note : Clip->LinkedNotes)
        {
            const float Start = TickToPixel(Note.StartTick);
            if (Start > AllottedGeometry.Size.X) continue;

			const float End = TickToPixel(Note.EndTick);
            if (End < 0) continue;

            const float Width = End - Start;
            if (Width < 0.1f) continue;

            const float Y = (127 - Note.Pitch) * RowHeight + Position.Get().Y; // Add vertical offset

            // Fix deprecated API
            FSlateDrawElement::MakeBox(
                OutDrawElements,
                LayerId,
                AllottedGeometry.ToPaintGeometry(
                    FVector2f(Width, RowHeight),
                    FSlateLayoutTransform(FVector2f(Start, Y))
                ),
                NoteBrush,
                ESlateDrawEffect::None,
                TrackColorRef
            );
        }

        // Paint timeline and play cursor
        LayerId = PaintTimeline(Args, AllottedGeometry, MyCullingRect, OutDrawElements, ++LayerId);
        LayerId = PaintPlayCursor(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
    }

    // Debug text
    const auto ToPrint = FText::FromString(FString::Printf(TEXT("Track %d\nVZoom: %f\n HZoom: %f\n Offset (%f, %f) \n TimeLineHeight %f\nFollow Cursor: %s"),
        TrackIndex, Zoom.Get().X, Zoom.Get().Y, Position.Get().X, Position.Get().Y, TimelineHeight,
        bFollowCursor.Get() ? TEXT("ON") : TEXT("OFF")));

    FSlateDrawElement::MakeText(
        OutDrawElements,
        LayerId++,
        AllottedGeometry.ToPaintGeometry(),
        ToPrint,
        FAppStyle::GetFontStyle("NormalFont"),
        ESlateDrawEffect::None,
        FLinearColor::White
    );

    return LayerId;
}

int32 SMidiClipVelocityEditor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    LayerId = PaintTimeline(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
	const FLinearColor& TrackColorRef = SequenceData->GetTrackMetadata(TrackIndex).TrackColor;

    if (Clip != nullptr)
    {
        // Draw sustain pedal events
        LayerId++;
        FMidiSustainPedalEvent CurrentSustainOnEvent;

        for (const auto& SustainEvent : Clip->SustainPedalEvents)
        {
            if (SustainEvent.bIsSustainPedalDown)
            {
                CurrentSustainOnEvent = SustainEvent;
            }
            else
            {
                const float Start = TickToPixel(CurrentSustainOnEvent.StartTick);
                const float End = TickToPixel(SustainEvent.StartTick);
                const float Width = End - Start;

                // Fix deprecated API
                FSlateDrawElement::MakeBox(
                    OutDrawElements,
                    LayerId,
                    AllottedGeometry.ToPaintGeometry(
                        FVector2f(Width, AllottedGeometry.GetLocalSize().Y),
                        FSlateLayoutTransform(FVector2f(Start, 0))
                    ),
                    FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
                    ESlateDrawEffect::None,
                    FLinearColor(0.5f, 0.1f, 0.1f, 0.5f)
                );
            }
        }

        // Draw velocity lines and markers
        LayerId++;
        static const FSlateBrush* NoteBrush = FUndawStyle::Get().GetBrush("MidiNoteBrush.Selected");
        const float Rotate = FMath::DegreesToRadians(45.f);

        for (const auto& Note : Clip->LinkedNotes)
        {
            const float Start = TickToPixel(Note.StartTick);
            const float Y = ((127 - Note.NoteVelocity) / 127.0f) * AllottedGeometry.GetLocalSize().Y;

            // Velocity line
            FSlateDrawElement::MakeLines(
                OutDrawElements,
                LayerId,
                AllottedGeometry.ToPaintGeometry(),
                {
                    FVector2D(Start, AllottedGeometry.GetLocalSize().Y),
                    FVector2D(Start, Y)
                },
                ESlateDrawEffect::None,
                TrackColorRef
            );

            // Velocity marker
            FSlateDrawElement::MakeRotatedBox(
                OutDrawElements,
                LayerId,
                AllottedGeometry.ToPaintGeometry(
                    FVector2f(10, 10),
                    FSlateLayoutTransform(FVector2f(Start - 5, Y))
                ),
                NoteBrush,
                ESlateDrawEffect::None,
                Rotate,
                FVector2D::ZeroVector,
                FSlateDrawElement::RelativeToElement,
                TrackColorRef
            );
        }

        // Paint play cursor and debug info
        LayerId = PaintPlayCursor(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
    }

    return LayerId;
}

int32 SMidiEditorPanelBase::PaintTimeline(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	// Recalculate grid with current geometry to ensure we have up-to-date grid points
	const_cast<SMidiEditorPanelBase*>(this)->RecalculateGrid(&AllottedGeometry);

	const bool bShouldPaintTimelineBar = TimelineHeight > 0.0f;

	if (bShouldPaintTimelineBar)
	{
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(
				FVector2f(AllottedGeometry.Size.X - MajorTabWidth, TimelineHeight),
				FSlateLayoutTransform(FVector2f(MajorTabWidth, 0))
			),
			FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
			ESlateDrawEffect::None,
			FLinearColor::Black
		);
	}

	// Setup drawing parameters
	const auto* MidiSongMap = SequenceData->HarmonixMidiFile->GetSongMaps();
	const float Height = (AllottedGeometry.Size.Y / 127.0f) / Zoom.Get().Y;
	const FLinearColor BarLineColor = FLinearColor::Gray.CopyWithNewOpacity(0.1f);
	const FLinearColor BeatLineColor = FLinearColor::Blue.CopyWithNewOpacity(0.1f);
	const FLinearColor SubdivisionLineColor = FLinearColor::Black.CopyWithNewOpacity(0.05f);
	const FLinearColor BarTextColor = FLinearColor::Gray.CopyWithNewOpacity(0.5f);
	
	// Use the proper font constructor
	const FSlateFontInfo LargeFont = FCoreStyle::GetDefaultFontStyle("Regular", 14);
	const FSlateFontInfo SmallFont = FCoreStyle::GetDefaultFontStyle("Regular", 7);

	// Calculate pixels per bar for text density decisions
	const float TicksPerBar = MidiSongMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, 0);
	const float TicksPerMs = 1.0f / MidiSongMap->TickToMs(1.0f);
	const float PixelsPerTick = Zoom.Get().X / TicksPerMs;
	const float PixelsPerBar = PixelsPerTick * TicksPerBar;

	// Draw grid points
	for (const auto& [OriginTick, GridPoint] : GridPoints)
	{
		const float Tick = OriginTick - GetStartOffset();
		const float PixelPosition = TickToPixel(Tick);

		// Only draw timeline elements if they're at or to the right of the controls area
		const bool bShouldDrawTimelineElements = PixelPosition >= MajorTabWidth;

		switch (GridPoint.Type)
		{
		case UnDAW::EGridPointType::Bar:
			if (bShouldPaintTimelineBar && bShouldDrawTimelineElements)
			{
				// Check if density calculator allows this bar
				const bool bShouldShowBarText = UnDAW::FTimelineGridDensityCalculator::ShouldShowBarText(GridPoint.Bar, CurrentGridDensity, PixelsPerBar);

				// Only show bar text if density calculator says we should
				if (bShouldShowBarText)
				{
					// Bar number
					FSlateDrawElement::MakeText(
						OutDrawElements,
						LayerId + 1, // Ensure text is above background
						AllottedGeometry.ToPaintGeometry(
							FVector2f(50.0f, Height),
							FSlateLayoutTransform(FVector2f(PixelPosition, 0))
						),
						FText::FromString(FString::FromInt(GridPoint.Bar)),
						LargeFont,
						ESlateDrawEffect::None,
						BarTextColor
					);

					// Beat number
					FSlateDrawElement::MakeText(
						OutDrawElements,
						LayerId + 1,
						AllottedGeometry.ToPaintGeometry(
							FVector2f(50.0f, Height),
							FSlateLayoutTransform(FVector2f(PixelPosition, 18))
						),
						FText::FromString(FString::FromInt(GridPoint.Beat)),
						SmallFont,
						ESlateDrawEffect::None,
						FLinearColor::White
					);

					// Subdivision number
					FSlateDrawElement::MakeText(
						OutDrawElements,
						LayerId + 1,
						AllottedGeometry.ToPaintGeometry(
							FVector2f(50.0f, Height),
							FSlateLayoutTransform(FVector2f(PixelPosition, 30))
						),
						FText::FromString(FString::FromInt(GridPoint.Subdivision)),
						SmallFont,
						ESlateDrawEffect::None,
						FLinearColor::White
					);
				}
			}

			// Only draw bar lines if they're at or to the right of the controls area
			if (bShouldDrawTimelineElements)
			{
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 2, // Lines above background and text
					AllottedGeometry.ToPaintGeometry(),
					{
						FVector2D(PixelPosition, 0),
						FVector2D(PixelPosition, AllottedGeometry.GetLocalSize().Y)
					},
					ESlateDrawEffect::None,
					BarLineColor,
					false,
					1.0f
				);
			}
			break;

		case UnDAW::EGridPointType::Beat:
			// Only draw beat lines if they're at or to the right of the controls area
			if (bShouldDrawTimelineElements)
			{
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 2,
					AllottedGeometry.ToPaintGeometry(),
					{
						FVector2D(PixelPosition, 0),
						FVector2D(PixelPosition, AllottedGeometry.GetLocalSize().Y)
					},
					ESlateDrawEffect::None,
					BeatLineColor,
					false,
					1.0f
				);
			}
			break;

		case UnDAW::EGridPointType::Subdivision:
			// Only draw subdivision lines if they're at or to the right of the controls area
			if (bShouldDrawTimelineElements)
			{
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 2,
					AllottedGeometry.ToPaintGeometry(),
					{
						FVector2D(PixelPosition, 0),
						FVector2D(PixelPosition, AllottedGeometry.GetLocalSize().Y)
					},
					ESlateDrawEffect::None,
					SubdivisionLineColor,
					false,
					1.0f
				);
			}
			break;
		}
	}

	return LayerId + 3; // Incremented LayerId based on drawn elements
}

// Helper method to add beats and subdivisions for a bar
void SMidiEditorPanelBase::AddBeatsAndSubdivisions(float BarStartTick, int32 BarNumber, UnDAW::TimelineConstants::EGridDensity Density, const FSongMaps* SongsMap, float VisibleEndTick)
{
	const int32 TicksPerBeat = SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Beat, BarStartTick);
	const int32 TicksPerBar = SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, BarStartTick);
	
	// Assuming 4/4 time signature for now - could be made dynamic later
	const int32 BeatsPerBar = 4;
	
	for (int32 Beat = 1; Beat <= BeatsPerBar; Beat++)
	{
		const float BeatTick = BarStartTick + (Beat - 1) * TicksPerBeat;
		
		if (BeatTick > VisibleEndTick) break;
		if (BeatTick == BarStartTick) continue; // Skip beat 1 as it's the same as the bar
		
		const UnDAW::FMusicalGridPoint BeatGridPoint = {
			UnDAW::EGridPointType::Beat,
			BarNumber,
			static_cast<int8>(Beat),
			1
		};
		
		if (UnDAW::FTimelineGridDensityCalculator::ShouldShowGridPoint(BeatGridPoint, Density, BarNumber))
		{
			GridPoints.Add(BeatTick, BeatGridPoint);
			
			// Add subdivisions if density allows
			if (Density == UnDAW::TimelineConstants::EGridDensity::Subdivisions)
			{
				AddSubdivisions(BeatTick, BarNumber, Beat, SongsMap, VisibleEndTick);
			}
		}
	}
}

// Helper method to add subdivisions for a beat
void SMidiEditorPanelBase::AddSubdivisions(float BeatStartTick, int32 BarNumber, int32 BeatNumber, const FSongMaps* SongsMap, float VisibleEndTick)
{
	const int32 TicksPerBeat = SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Beat, BeatStartTick);
	
	// Assuming 16th note subdivisions (4 per beat)
	const int32 SubdivisionsPerBeat = 4;
	const int32 TicksPerSubdivision = TicksPerBeat / SubdivisionsPerBeat;
	
	for (int32 Subdivision = 1; Subdivision <= SubdivisionsPerBeat; Subdivision++)
	{
		const float SubdivisionTick = BeatStartTick + (Subdivision - 1) * TicksPerSubdivision;
		
		if (SubdivisionTick > VisibleEndTick) break;
		if (SubdivisionTick == BeatStartTick) continue; // Skip subdivision 1 as it's the same as the beat
		
		const UnDAW::FMusicalGridPoint SubdivisionGridPoint = {
			UnDAW::EGridPointType::Subdivision,
			BarNumber,
			static_cast<int8>(BeatNumber),
			static_cast<int8>(Subdivision)
		};
		
		GridPoints.Add(SubdivisionTick, SubdivisionGridPoint);
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION