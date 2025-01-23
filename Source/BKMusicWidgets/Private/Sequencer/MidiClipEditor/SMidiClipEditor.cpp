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

	//FSoftClassPath NoteBrushPath = FSoftClassPath(TEXT("/unDAW/Brushes/MidiNoteBrush.MidiNoteBrush"));
	//NoteBrush = NoteBrushPath.TryLoad();
	

	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/

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

    // Draw piano grid
    LayerId++;
    for (int i = 0; i < 128; i++)
    {
        const float Y = (127 - i) * RowHeight;

        // Draw grid background
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(
                FVector2D(0, Y),
                FVector2D(AllottedGeometry.Size.X, RowHeight)
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
                    FVector2D(0, Y),
                    FVector2D(50.0f, RowHeight)
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

            const float Y = (127 - Note.Pitch) * RowHeight;

            FSlateDrawElement::MakeBox(
                OutDrawElements,
                LayerId,
                AllottedGeometry.ToPaintGeometry(
                    FVector2D(Start, Y),
                    FVector2D(Width, RowHeight)
                ),
                NoteBrush,
                ESlateDrawEffect::None,
                TrackColor
            );
        }

        // Paint timeline and play cursor
        LayerId = PaintTimeline(Args, AllottedGeometry, MyCullingRect, OutDrawElements, ++LayerId);
        LayerId = PaintPlayCursor(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
    }

    // Debug text
    const auto ToPrint = FText::FromString(FString::Printf(TEXT("Track %d\nVZoom: %f\n HZoom: %f\n Offset (%f, %f)"),
        TrackIndex, Zoom.Get().X, Zoom.Get().Y, Position.Get().X, Position.Get().Y));
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

                FSlateDrawElement::MakeBox(
                    OutDrawElements,
                    LayerId,
                    AllottedGeometry.ToPaintGeometry(
                        FVector2D(Start, 0),
                        FVector2D(Width, AllottedGeometry.GetLocalSize().Y)
                    ),
                    FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
                    ESlateDrawEffect::None,
                    FLinearColor(0.1f, 0.1f, 0.1f, 0.01f)
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
                TrackColor
            );

            // Velocity marker
            FSlateDrawElement::MakeRotatedBox(
                OutDrawElements,
                LayerId,
                AllottedGeometry.ToPaintGeometry(
                    FVector2D(Start - 5, Y),
                    FVector2D(10, 10)
                ),
                NoteBrush,
                ESlateDrawEffect::None,
                Rotate,
                TOptional<FVector2D>(),
                FSlateDrawElement::RelativeToElement,
                TrackColor
            );
        }

        // Paint play cursor and debug info
        LayerId = PaintPlayCursor(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);

        const auto ToPrint = FText::FromString(FString::Printf(TEXT("Track %d\nVZoom: %f\n HZoom: %f\n Offset (%f, %f)\n Bar, Beat: %d, %f"),
            TrackIndex, Zoom.Get().X, Zoom.Get().Y, Position.Get().X, Position.Get().Y, PlayCursor.Get().Bar, PlayCursor.Get().Beat));
        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.ToPaintGeometry(),
            ToPrint,
            FAppStyle::GetFontStyle("NormalFont"),
            ESlateDrawEffect::None,
            FLinearColor::White
        );
    }

    return LayerId;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION