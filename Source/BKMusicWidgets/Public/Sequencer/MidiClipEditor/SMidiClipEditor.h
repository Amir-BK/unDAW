// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Events.h"
#include "Widgets/SCompoundWidget.h"
#include "SlateFwd.h"
#include "Components/Widget.h"



/**
 * 
 */
class BKMUSICWIDGETS_API SMidiClipEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMidiClipEditor)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

};
