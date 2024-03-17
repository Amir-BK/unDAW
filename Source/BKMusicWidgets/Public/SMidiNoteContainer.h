// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Misc/Attribute.h"
#include <SPianoRollGraph.h>

/**
 * 
 */
class SMidiNoteContainer : public SLeafWidget
{
	SLATE_DECLARE_WIDGET_API(SMidiNoteContainer, SLeafWidget, BKMUSICWIDGETS_API)

public:
	SLATE_BEGIN_ARGS(SMidiNoteContainer)
		: _Color(FLinearColor::White)
		,_AlphaBackgroundBrush(FAppStyle::Get().GetBrush("ColorPicker.AlphaBackground"))
		, _CornerRadius(0.0f)
		, _ColorIsHSV(false)
		, _ShowBackgroundForAlpha(false)
		, _UseSRGB(true)
		, _AlphaDisplayMode(EColorBlockAlphaDisplayMode::Combined)
		, _Size(FVector2D(16, 16))
		, _UIDinParent(-1)
		//, _ParentGraph(nullptr)
	{}
		/** The color to display for this color block */
		SLATE_ATTRIBUTE(FLinearColor, Color)

		/** Background to display for when there is a color with transparency. Irrelevant if ignoring alpha */
		SLATE_ATTRIBUTE(const FSlateBrush*, AlphaBackgroundBrush)

		/** Rounding to apply to the corners of the block */
		SLATE_ATTRIBUTE(FVector4, CornerRadius)

		/** Whether the color displayed is HSV or not */
		SLATE_ATTRIBUTE(bool, ColorIsHSV)

		/** Whether to display a background for viewing opacity. Irrelevant if ignoring alpha */
		SLATE_ATTRIBUTE(bool, ShowBackgroundForAlpha)

		/** Whether to display sRGB color */
		SLATE_ATTRIBUTE(bool, UseSRGB)

		/** How the color block displays color and opacity */
		SLATE_ATTRIBUTE(EColorBlockAlphaDisplayMode, AlphaDisplayMode)

		/** How big should this color block be? */
		SLATE_ATTRIBUTE(FVector2D, Size)


		
		/** Identifier in parent map */
		SLATE_ATTRIBUTE(int32, UIDinParent)

	SLATE_END_ARGS()

	SMidiNoteContainer();

	FLinearColor NoteColor;
	private:
	//TSlateAttribute<FSlateColor> NoteColorAttribute;

	public:
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	FSlateColor GetNoteColor() const {
		return FSlateColor::UseForeground();// NoteColorAttribute.Get();
	}

	void SetNoteColor(TAttribute<FSlateColor> InColor){
		//NoteColorAttribute.Assign(*this, InColor);
	}

	void SetParentPointer(SPianoRollGraph* inParentGraph);
	void SetParentSharedPtr(TSharedPtr<SPianoRollGraph> inParentGraph);
	void CallOnPressed();
	void CallOnVerticalMove(int rowsDelta);

	TEnumAsByte<EButtonClickMethod::Type> GetClickMethodFromInputType(const FPointerEvent& MouseEvent) const;

	//FMidiNoteProperties containedNote; 
	TSharedPtr<SPianoRollGraph> parentPianoRollPanel; 
	SPianoRollGraph* parentGraph;
	TAttribute<int32> UniqueIDinParent;
	FSlateBrush* BrushResource;

	TOptional<EMouseCursor::Type> currentCursor;
	//TSharedRef<SPianoRollGraph> parentSharedRef;

	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	
	void MakeSection(TArray<FSlateGradientStop>& OutGradientStops, FVector2D StartPt, FVector2D EndPt, FLinearColor Color, const FWidgetStyle& InWidgetStyle, bool bIgnoreAlpha) const;
	
	virtual FVector2D ComputeDesiredSize(float) const override;
	//EVisibility GetVisibility() override

	/** Sets whether a click should be triggered on mouse down, mouse up, or that both a mouse down and up are required. */
	TEnumAsByte<EButtonClickMethod::Type> ClickMethod;

	/** How should the button be clicked with touch events? */
	TEnumAsByte<EButtonTouchMethod::Type> TouchMethod;

	/** How should the button be clicked with keyboard/controller button events? */
	TEnumAsByte<EButtonPressMethod::Type> PressMethod;
	
	//SMidiNoteContainer();

	TOptional<EMouseCursor::Type> GetCursor() const override
	{
		return currentCursor;
	};


		private:
			/** The color to display for this color block */
			TSlateAttribute<FLinearColor> Color;

			TSlateAttribute<const FSlateBrush*> AlphaBackgroundBrush;

			TSlateAttribute<FVector4> GradientCornerRadius;

			TSlateAttribute<FVector2D> ColorBlockSize;

			/** A handler to activate when the mouse is pressed. */
			FPointerEventHandler MouseButtonDownHandler;

			/** Whether to ignore alpha entirely from the input color */
			TSlateAttribute<EColorBlockAlphaDisplayMode> AlphaDisplayMode;

			/** Whether the color displayed is HSV or not */
			TSlateAttribute<bool> ColorIsHSV;

			/** Whether to display a background for viewing opacity. Irrelevant if ignoring alpha */
			TSlateAttribute<bool> ShowBackgroundForAlpha;

			/** Whether to display sRGB color */
			TSlateAttribute<bool> bUseSRGB;
};
