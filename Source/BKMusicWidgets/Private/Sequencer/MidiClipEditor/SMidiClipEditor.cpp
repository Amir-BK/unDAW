// Fill out your copyright notice in the Description page of Project Settings.


#include "Sequencer/MidiClipEditor/SMidiClipEditor.h"
#include "SlateOptMacros.h"
#include "Styling/AppStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMidiClipEditor::Construct(const FArguments& InArgs, UDAWSequencerData* InSequence)
{
	SequenceData = InSequence;
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

		ZoomToContent();

	}
}
inline int32 SMidiClipEditor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	auto ScreenCenterPaintGeometry = AllottedGeometry.ToOffsetPaintGeometry(FVector2D(AllottedGeometry.Size / 2.0f));
	auto OffsetPaintGeometry = AllottedGeometry.ToOffsetPaintGeometry(FVector2D(HorizontalOffset, VerticalOffset));

	PaintTimelineMarks(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	//draw notes
	const float Height = (AllottedGeometry.Size.Y / 127) / VerticalZoom;
	if (Clip != nullptr)
	{
		LayerId++;
		for (const auto& Note : Clip->LinkedNotes)
		{
			const float Start = TickToPixel(Note.StartTick + Clip->StartTick);
			const float End = TickToPixel(Note.EndTick + Clip->StartTick);
			const float Width = End - Start;
			const float Y = (127 - Note.Pitch) * Height;

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				OffsetPaintGeometry,
				{ FVector2D(Start, Y), FVector2D(Start + Width, Y) },
				ESlateDrawEffect::None,
				TrackColor
			);
		}

	}

	const auto ToPrint = FText::FromString(FString::Printf(TEXT("Track %d\nVZoom: %f\n HZoom: %f\n Offset (%f, %f)"), TrackIndex, HorizontalZoom, VerticalZoom, HorizontalOffset, VerticalOffset));
	FSlateDrawElement::MakeText(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), ToPrint, FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::White);

	return LayerId;
}

inline int32 SMidiEditorPanelBase::PaintTimelineMarks(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//paint some test text for sanity check
	//auto ToPrint = FText::FromString("Hello World");
	//::MakeText(OutDrawElements, LayerId++, AllottedGeometry, ToPrint, FAppStyle::GetFontStyle("Font.Large.Bold"), ESlateDrawEffect::None, FLinearColor::White);
	auto OffsetGeometryChild = AllottedGeometry;
	const auto* MidiSongMap = SequenceData->HarmonixMidiFile->GetSongMaps();
	using namespace UnDAW;
	const float Height = (AllottedGeometry.Size.Y / 127) / VerticalZoom;

	for (const auto& [Tick, GridPoint] : GridPoints)
	{
		FLinearColor LineColor;

		//ugly as heck but probably ok for now
		switch (GridPoint.Type)
		{
		case EGridPointType::Bar:
			//draw bar number
			FSlateDrawElement::MakeText(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -VerticalOffset))),
				FText::FromString(FString::FromInt(GridPoint.Bar)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 14),
				ESlateDrawEffect::None,
				FLinearColor::White
			);

			//draw beat number
			FSlateDrawElement::MakeText(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -VerticalOffset + 18))),
				FText::FromString(FString::FromInt(GridPoint.Beat)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
				ESlateDrawEffect::None,
				FLinearColor::White
			);

			//draw subdivision number
			FSlateDrawElement::MakeText(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -VerticalOffset + 30))),
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
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -VerticalOffset + 30))),
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
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -VerticalOffset + 18))),
				FText::FromString(FString::FromInt(GridPoint.Beat)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
				ESlateDrawEffect::None,
				FLinearColor::White
			);

			//draw subdivision number
			FSlateDrawElement::MakeText(OutDrawElements,
				LayerId,
				OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -VerticalOffset + 30))),
				FText::FromString(FString::FromInt(GridPoint.Subdivision)),
				FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
				ESlateDrawEffect::None,
				FLinearColor::White
			);

			LineColor = FLinearColor::Blue;
			break;


		}


	}
	return LayerId;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION