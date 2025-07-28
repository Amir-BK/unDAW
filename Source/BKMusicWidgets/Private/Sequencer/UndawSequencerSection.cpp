#include "Sequencer/UndawSequencerSection.h"
#include "SlateFwd.h"
#include "Components/Widget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"



void SDawSequencerTrackMidiSection::Construct(const FArguments& InArgs, FLinkedNotesClip* InClip, FDawSequencerTrack* InParentTrack, UDAWSequencerData* InSequenceToEdit)
{
    Clip = InClip;
    TrackColor = InArgs._TrackColor;
    ParentTrack = InParentTrack;
    Position = InArgs._Position;
    Zoom = InArgs._Zoom;
    SequenceData = InSequenceToEdit;
}

float SDawSequencerTrackMidiSection::CalculateXPosition(float Tick) const
{
    return TickToPixel(Tick);
}

int32 SDawSequencerTrackMidiSection::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const bool bIsHoveredStrong = bIsHovered && GetParentWidget()->IsHovered();
    const FLinearColor ColorToUse = bIsSelected ? TrackColor.Get().CopyWithNewOpacity(0.5f) : bIsHoveredStrong ? TrackColor.Get().CopyWithNewOpacity(0.2f) : TrackColor.Get().CopyWithNewOpacity(0.1f);

    // Paint the background box using the full allocated geometry (which should already be sized correctly by the parent)
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId++,
        AllottedGeometry.ToPaintGeometry(),
        FAppStyle::GetBrush("Sequencer.Section.Background_Contents"),
        ESlateDrawEffect::None,
        ColorToUse
    );

    // Draw notes
    LayerId++;
    const float Height = AllottedGeometry.Size.Y / 127;
    for (const auto& Note : Clip->LinkedNotes)
    {
        // Calculate note positions relative to the clip's start
        const float StartMs = SequenceData->HarmonixMidiFile->GetSongMaps()->TickToMs(Note.StartTick);
        const float EndMs = SequenceData->HarmonixMidiFile->GetSongMaps()->TickToMs(Note.EndTick);
        
        const float X = StartMs * Zoom.Get().X;
        const float Width = (EndMs - StartMs) * Zoom.Get().X;
        const float Y = (127 - Note.Pitch) * Height;

        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(
                FVector2f(X, Y),
                FVector2f(Width, Height)
            ),
            FAppStyle::GetBrush("WhiteBrush"),
            ESlateDrawEffect::None,
            FLinearColor::White
        );
    }

    return LayerId;
}

const float SDawSequencerTrackMidiSection::TickToPixel(const float Tick) const
{
    if (!SequenceData || !SequenceData->HarmonixMidiFile)
    {
        return 0.0f;
    }

    const float Milliseconds = SequenceData->HarmonixMidiFile->GetSongMaps()->TickToMs(Tick);
    // Apply zoom and position offset consistently
    return (Milliseconds * Zoom.Get().X) + Position.Get().X;
}