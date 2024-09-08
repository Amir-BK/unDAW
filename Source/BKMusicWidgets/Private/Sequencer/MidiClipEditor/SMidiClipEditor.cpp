// Fill out your copyright notice in the Description page of Project Settings.


#include "Sequencer/MidiClipEditor/SMidiClipEditor.h"
#include "SlateOptMacros.h"
#include "Styling/AppStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMidiClipEditor::Construct(const FArguments& InArgs)
{
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
}
int32 SMidiClipEditor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// draw 'hello' in the center of the widget
	auto ScreenCenterPaintGeometry = AllottedGeometry.ToPaintGeometry(FVector2D(AllottedGeometry.Size / 2.0f), 1.0f);

	FSlateDrawElement::MakeText(OutDrawElements, LayerId, ScreenCenterPaintGeometry, FText::FromString("Hello"), FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::White);

	return LayerId;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
