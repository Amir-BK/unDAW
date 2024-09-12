// Fill out your copyright notice in the Description page of Project Settings.


#include "Sequencer/MidiClipEditor/SMidiClipEditor.h"
#include "SlateOptMacros.h"


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
	auto OffsetGeometryChild = AllottedGeometry.MakeChild(AllottedGeometry.GetLocalSize(), FSlateLayoutTransform(1.0f, Position.Get()));

	auto TrackAreaGeometry = AllottedGeometry.MakeChild(
		FVector2f(0, TimelineHeight),
		FVector2f(AllottedGeometry.Size.X, AllottedGeometry.Size.Y - TimelineHeight)
	);

	
	//PaintTimelineMarks(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	//draw notes
	TArray<FSlateGradientStop> GradientStops = { FSlateGradientStop(FVector2D(0,0), TrackColor) };
	if (Clip != nullptr)
	{
		LayerId++;
		for (const auto& Note : Clip->LinkedNotes)
		{
			const float Start = TickToPixel(Note.StartTick);
			if (Start > AllottedGeometry.Size.X - Position.Get().X) continue;
			

			const float End = TickToPixel(Note.EndTick);
			if (End < 0 - Position.Get().X) continue;
			const float Width = End - Start;

			if (Width < 0.1) continue;

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
				GradientStops, EOrientation::Orient_Horizontal, ESlateDrawEffect::NoGamma,
				FVector4f::One() * GradientStrength);

			//surround note with outline
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(Width, RowHeight), FSlateLayoutTransform(1.0f, FVector2D(Start, Y))),
				{ FVector2D(0, 0), FVector2D(Width, 0), FVector2D(Width, RowHeight), FVector2D(0, RowHeight), FVector2D(0, 0) },
				ESlateDrawEffect::NoGamma,
				FLinearColor::White, true, 2.0f
			);

			//FSlateDrawElement::MakeBox




		}

		PaintTimeline(Args, AllottedGeometry, MyCullingRect, OutDrawElements, ++LayerId);

	}

	const auto ToPrint = FText::FromString(FString::Printf(TEXT("Track %d\nVZoom: %f\n HZoom: %f\n Offset (%f, %f)"), TrackIndex, Zoom.Get().X, Zoom.Get().Y, Position.Get().X, Position.Get().Y));
	FSlateDrawElement::MakeText(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), ToPrint, FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::White);

	return LayerId;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

int32 SMidiClipVelocityEditor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (Clip != nullptr)
	{
		auto OffsetGeometryChild = AllottedGeometry.MakeChild(AllottedGeometry.GetLocalSize(), FSlateLayoutTransform(1.0f, Position.Get()));
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

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(1, AllottedGeometry.GetLocalSize().Y), FSlateLayoutTransform(1.0f, FVector2D(Start, 0))),
				{ FVector2D(0, AllottedGeometry.GetLocalSize().Y), FVector2D(0, Y) },
				ESlateDrawEffect::None,
				TrackColor
			);

			//FSlateDrawElement::MakeBox(
			//	OutDrawElements,
			//	LayerId,
			//	OffsetGeometryChild.ToPaintGeometry(FVector2D(1, AllottedGeometry.GetLocalSize().Y), FSlateLayoutTransform(1.0f, FVector2D(Start, AllottedGeometry.GetLocalSize().Y - Y))),
			//	SolidKeyBarBrush,
			//	ESlateDrawEffect::None,
			//	TrackColor
			//);

			//paint the velcity value as text
			FSlateDrawElement::MakeText(OutDrawElements, LayerId++, OffsetGeometryChild.ToOffsetPaintGeometry(FVector2D(Start, Y)), FText::FromString(FString::Printf(TEXT("%d"), Note.NoteVelocity)), FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, TrackColor);

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

		const auto ToPrint = FText::FromString(FString::Printf(TEXT("Track %d\nVZoom: %f\n HZoom: %f\n Offset (%f, %f)"), TrackIndex, Zoom.Get().X, Zoom.Get().Y, Position.Get().X, Position.Get().Y));
		FSlateDrawElement::MakeText(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), ToPrint, FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::White);

	}

	
	return LayerId;
}
