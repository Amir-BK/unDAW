#include "Sequencer/UndawSequencerSection.h"
#include "SlateFwd.h"
#include "Components/Widget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"



void SDawSequencerTrackMidiSection::Construct(const FArguments& InArgs, FLinkedNotesClip* InClip, FDawSequencerTrack* InParentTrack)
{
	Clip = InClip;
	TrackColor = InArgs._TrackColor;
	ParentTrack = InParentTrack;
	Position = InArgs._Position;
	Zoom = InArgs._Zoom;
}

int32 SDawSequencerTrackMidiSection::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//print section height cause wtf
	//UE_LOG(LogTemp, Warning, TEXT("Section Height %f"), AllottedGeometry.Size.Y);
	const float SectionDuration = (Clip->EndTick - Clip->StartTick) * Zoom.Get().X;
	//just fill the background with a gray box

	const bool bIsHoveredStrong = bIsHovered && GetParentWidget()->IsHovered();
	const FLinearColor ColorToUse = bIsSelected ? TrackColor.Get().CopyWithNewOpacity(0.5f) : bIsHoveredStrong ? TrackColor.Get().CopyWithNewOpacity(0.2f) : TrackColor.Get().CopyWithNewOpacity(0.1f);

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId++,
		AllottedGeometry.ToPaintGeometry(FVector2D(SectionDuration, AllottedGeometry.Size.Y), 1.0f),
		FAppStyle::GetBrush("Sequencer.Section.Background_Contents"),
		ESlateDrawEffect::None,
		ColorToUse
	);



	//draw notes
	const float Height = AllottedGeometry.Size.Y / 127;
	LayerId++;
	for (const auto& Note : Clip->LinkedNotes)
	{
		const float X = (Note.StartTick);
		const float Width = (Note.EndTick - Note.StartTick);
		const float Y = (127 - Note.Pitch) * Height;

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			{ FVector2D(X, Y), FVector2D(X + Width, Y) },
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}

	// draw start and end ticks it helps with debugging
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
