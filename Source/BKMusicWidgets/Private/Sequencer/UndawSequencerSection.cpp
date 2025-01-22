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
    return TickToPixel(Tick + Clip->OffsetTick) - Position.Get().X * Zoom.Get().X;
}

int32 SDawSequencerTrackMidiSection::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const bool bIsHoveredStrong = bIsHovered && GetParentWidget()->IsHovered();
    const FLinearColor ColorToUse = bIsSelected ? TrackColor.Get().CopyWithNewOpacity(0.5f) : bIsHoveredStrong ? TrackColor.Get().CopyWithNewOpacity(0.2f) : TrackColor.Get().CopyWithNewOpacity(0.1f);

    // Calculate the geometry for the background box
    const float BackgroundX = CalculateXPosition(Clip->StartTick);
    const float BackgroundWidth = TickToPixel(Clip->EndTick + Clip->OffsetTick) - TickToPixel(Clip->StartTick + Clip->OffsetTick);
    FGeometry BackgroundGeometry = AllottedGeometry.MakeChild(
        FVector2f(BackgroundX, 0),
        FVector2f(BackgroundWidth, AllottedGeometry.Size.Y)
    );

    // Paint the background box
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
        // Calculate relative to the clip's start position

        const float X = TickToPixel(Note.StartTick + Clip->StartTick) - BackgroundX;
        const float End = TickToPixel(Note.EndTick + Clip->StartTick) - BackgroundX;
		const float Width = End - X;
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

    // Draw start and end ticks for debugging
    FSlateDrawElement::MakeText(
        OutDrawElements,
        LayerId++,
        AllottedGeometry.ToPaintGeometry(),
        FText::FromString(FString::Printf(TEXT("Start: %d, Stop: %d\n Position: %s, Zoom: %s"), Clip->StartTick, Clip->EndTick, *Position.Get().ToString(), *Zoom.Get().ToString())),
        FAppStyle::GetFontStyle("NormalFont"),
        ESlateDrawEffect::None,
        FLinearColor::White
    );

    return LayerId;
}

const float SDawSequencerTrackMidiSection::TickToPixel(const float Tick) const
{
    // Make sure SequenceData is valid
    if (!SequenceData || !SequenceData->HarmonixMidiFile)
    {
        return 0.0f;
    }

    const float Milliseconds = SequenceData->HarmonixMidiFile->GetSongMaps()->TickToMs(Tick);
    return Milliseconds * Zoom.Get().X;
}