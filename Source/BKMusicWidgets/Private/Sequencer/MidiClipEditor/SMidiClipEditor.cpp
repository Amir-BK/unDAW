// Fill out your copyright notice in the Description page of Project Settings.


#include "Sequencer/MidiClipEditor/SMidiClipEditor.h"
#include "SlateOptMacros.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMidiClipEditor::Construct(const FArguments& InArgs, UDAWSequencerData* InSequence)
{
	SMidiEditorPanelBase::Construct(SMidiEditorPanelBase::FArguments(), InSequence);
	SequenceData = InSequence;
	MajorTabWidth = 0.0f;
	TimelineHeight = InArgs._TimelineHeight;
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
}
void SMidiClipEditor::OnClipsFocused(TArray<TTuple<FDawSequencerTrack*, FLinkedNotesClip*>> Clips)
{
	if (!Clips.IsEmpty())
	{
		TrackIndex = Clips[0].Key->MetadataIndex;
		TrackMetaDataName = FText::FromString(SequenceData->GetTrackMetadata(TrackIndex).TrackName);
		Clip = Clips[0].Value;
		TrackColor = SequenceData->GetTrackMetadata(TrackIndex).TrackColor;

		//ZoomToContent();

	}
}
inline int32 SMidiClipEditor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	auto ScreenCenterPaintGeometry = AllottedGeometry.ToOffsetPaintGeometry(FVector2D(AllottedGeometry.Size / 2.0f));
	auto OffsetGeometryChild = AllottedGeometry.MakeChild(AllottedGeometry.GetLocalSize(), FSlateLayoutTransform(1.0f, FVector2D(HorizontalOffset, VerticalOffset)));

	auto TrackAreaGeometry = AllottedGeometry.MakeChild(
		FVector2f(0, TimelineHeight),
		FVector2f(AllottedGeometry.Size.X, AllottedGeometry.Size.Y - TimelineHeight)
	);

	LayerId = PaintTimeline(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
	//PaintTimelineMarks(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	//draw notes
	TArray<FSlateGradientStop> GradientStops = { FSlateGradientStop(FVector2D(0,0), TrackColor) };
	if (Clip != nullptr)
	{
		LayerId++;
		for (const auto& Note : Clip->LinkedNotes)
		{
			const float Start = TickToPixel(Note.StartTick);
			const float End = TickToPixel(Note.EndTick);
			const float Width = End - Start;
			const float Y = (127 - Note.Pitch) * RowHeight;

			//FSlateDrawElement::MakeLines(
			//	OutDrawElements,
			//	LayerId,
			//	OffsetPaintGeometry,
			//	{ FVector2D(Start, Y), FVector2D(Start + Width, Y) },
			//	ESlateDrawEffect::None,
			//	TrackColor
			//);

			const float GradientStrength = Width < 10 ? 0.0f : 2.0f;

			FSlateDrawElement::MakeGradient(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(Width, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(Start, Y))),
				GradientStops, EOrientation::Orient_Horizontal, ESlateDrawEffect::None,
				FVector4f::One() * GradientStrength);
		}

	}

	const auto ToPrint = FText::FromString(FString::Printf(TEXT("Track %d\nVZoom: %f\n HZoom: %f\n Offset (%f, %f)"), TrackIndex, HorizontalZoom, VerticalZoom, HorizontalOffset, VerticalOffset));
	FSlateDrawElement::MakeText(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), ToPrint, FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::White);

	return LayerId;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

int32 SMidiClipVelocityEditor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (Clip != nullptr)
	{
		auto OffsetGeometryChild = AllottedGeometry.MakeChild(AllottedGeometry.GetLocalSize(), FSlateLayoutTransform(1.0f, FVector2D(HorizontalOffset, VerticalOffset)));
		LayerId++;
		for (const auto& Note : Clip->LinkedNotes)
		{
			const float Start = TickToPixel(Note.StartTick);
			
			const float Y = (Note.NoteVelocity / 127.0f) * AllottedGeometry.GetLocalSize().Y;

			//paint a line extending from the bottom of the panel to the Y velocity value

			static const FSlateBrush* DottedKeyBarBrush = FAppStyle::GetBrush("Sequencer.KeyBar.Dotted");
			static const FSlateBrush* DashedKeyBarBrush = FAppStyle::GetBrush("Sequencer.KeyBar.Dashed");

			static const FSlateBrush* SolidKeyBarBrush = FAppStyle::GetBrush("Sequencer.KeyBar.Solid");

			//FSlateDrawElement::MakeLines(
			//	OutDrawElements,
			//	LayerId,
			//	AllottedGeometry.ToOffsetPaintGeometry(FVector2D(Start, 0)),
			//	{ FVector2D(Start, AllottedGeometry.GetLocalSize().Y), FVector2D(Start, Y) },
			//	ESlateDrawEffect::None,
			//	TrackColor
			//);

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(1, AllottedGeometry.GetLocalSize().Y), FSlateLayoutTransform(1.0f, FVector2D(Start, 0))),
				SolidKeyBarBrush,
				ESlateDrawEffect::None,
				TrackColor
			);

			//paint the velcity value as text
			FSlateDrawElement::MakeText(OutDrawElements, LayerId++, AllottedGeometry.ToOffsetPaintGeometry(FVector2D(Start, Y)), FText::FromString(FString::Printf(TEXT("%d"), Note.NoteVelocity)), FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::Black);

			//paint a small circle at the end of the line
		/*	FSlateDrawElement::Make(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToOffsetPaintGeometry(FVector2D(Start, Y)),
				5.0f,
				12,
				ESlateDrawEffect::None,
				TrackColor
			);*/

			//FSlateDrawElement::MakeText(OutDrawElements, LayerId++, AllottedGeometry.ToOffsetPaintGeometry(FVector2D(Start, Y)), FText::FromString(FString::Printf(TEXT("%d"), Note.Velocity)), FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::Black);
		}

		const auto ToPrint = FText::FromString(FString::Printf(TEXT("Track %d\nVZoom: %f\n HZoom: %f\n Offset (%f, %f)"), TrackIndex, HorizontalZoom, VerticalZoom, HorizontalOffset, VerticalOffset));
		FSlateDrawElement::MakeText(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), ToPrint, FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::White);

	}

	
	return LayerId;
}
