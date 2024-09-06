#include "UndawMusicSequencer.h"
#include "SlateFwd.h"
#include "Components/Widget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"


void SUndawMusicSequencer::Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit)
{
	SequenceData = InSequenceToEdit;

	const auto& SequenceName = SequenceData->GetFName();

	TimelineHeight = InArgs._TimelineHeight;

	ChildSlot[
		SAssignNew(ScrollBox, SScrollBox)
	];

	//CreateGridPanel();
	CreateScrollBox();
}

void SUndawMusicSequencer::CreateGridPanel()
{
	constexpr int TracksRow = 0;
	constexpr int LanesRow = 1;


	
	for (int i = 0; i < 20; i++)
	{
		SGridPanel::FSlot* TrackSlot;
		SGridPanel::FSlot* LaneSlot;

		GridPanel->AddSlot(TracksRow, i)
			.Expose(TrackSlot)
			[
				SNew(SBox)
					.WidthOverride(100)
					.HeightOverride(100)
					[
						SNew(SBorder)
							.BorderBackgroundColor(FLinearColor::Red)
							.Content()
							[
								SNew(STextBlock)
									.Text(FText::FromString(FString::Printf(TEXT("Track %d"), i)))
							]
					]

			];

		GridPanel->AddSlot(LanesRow, i)
			.Expose(LaneSlot)
			[
				SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("Track %d"), i)))
			];

	

		TrackSlots.Add(TrackSlot);
		LaneSlots.Add(LaneSlot);

	}
}

void SUndawMusicSequencer::CreateScrollBox()
{
	ScrollBox->ClearChildren();
	
	//add 25 tracks
	for (int i = 0; i < 25; i++)
	{
		SScrollBox::FSlot* TrackSlot;
		
		ScrollBox->AddSlot()
			.Expose(TrackSlot)
			[
				SNew(SBox)
					.HeightOverride(100)
					[
						SNew(SBorder)
							.BorderBackgroundColor(FLinearColor::Red)
							.Content()
							[
								SNew(STextBlock)
									.Text(FText::FromString(FString::Printf(TEXT("Track %d"), i)))
							]
					]
			];

		TrackSlotsScrollBox.Add(TrackSlot);
	}

}

int32 SUndawMusicSequencer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	auto TimelineGeometry = AllottedGeometry.MakeChild(
		FVector2f(0, 0),
		FVector2f(AllottedGeometry.Size.X, TimelineHeight)
	);

	auto TimelineCullingRect = MyCullingRect.IntersectionWith(FSlateRect::FromPointAndExtent(TimelineGeometry.LocalToAbsolute(FVector2D(0, 0)), TimelineGeometry.Size));

	auto TrackAreaGeometry = AllottedGeometry.MakeChild(
		FVector2f(0, TimelineHeight),
		FVector2f(AllottedGeometry.Size.X, AllottedGeometry.Size.Y - TimelineHeight)
	);

	
	LayerId = PaintTimeline(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
	LayerId = PaintBackgroundGrid(Args, TrackAreaGeometry, MyCullingRect, OutDrawElements, LayerId);
	LayerId = ScrollBox->Paint(Args, TrackAreaGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	
	return LayerId;
}

int32 SUndawMusicSequencer::PaintBackgroundGrid(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	//just fill the background with a gray box
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
		ESlateDrawEffect::None,
		FLinearColor::Green.CopyWithNewOpacity(0.2f)
	);
	
	return LayerId;
}

int32 SUndawMusicSequencer::PaintTimeline(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	// first draw the timeline backgroumd a black box

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
		ESlateDrawEffect::None,
		FLinearColor::Black
	);

	// draw 30 vertical lines for fun, 
	for (int i = 0; i < 30; i++)
	{
		const float X = i * 100;
		const FVector2D Start(X, 0);
		const FVector2D End(X, TimelineHeight);

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			{ Start, End },
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}

	
	return LayerId;
}

