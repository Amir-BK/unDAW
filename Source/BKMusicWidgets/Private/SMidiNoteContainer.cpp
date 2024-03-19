// Fill out your copyright notice in the Description page of Project Settings.


#include "SMidiNoteContainer.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLATE_IMPLEMENT_WIDGET(SMidiNoteContainer)

void SMidiNoteContainer::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, Color, EInvalidateWidgetReason::Paint);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, AlphaBackgroundBrush, EInvalidateWidgetReason::Paint);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, GradientCornerRadius, EInvalidateWidgetReason::Paint);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, ColorBlockSize, EInvalidateWidgetReason::Layout);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, AlphaDisplayMode, EInvalidateWidgetReason::Paint);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, ColorIsHSV, EInvalidateWidgetReason::Paint);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, ShowBackgroundForAlpha, EInvalidateWidgetReason::Paint);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, bUseSRGB, EInvalidateWidgetReason::Paint);
}

SMidiNoteContainer::SMidiNoteContainer()
	: Color(*this, FLinearColor::White)
	, AlphaBackgroundBrush(*this, nullptr)
	, GradientCornerRadius(*this, FVector4(0.0f))
	, ColorBlockSize(*this, FVector2D(16, 16))
	, AlphaDisplayMode(*this, EColorBlockAlphaDisplayMode::Combined)
	, ColorIsHSV(*this, false)
	, ShowBackgroundForAlpha(*this, false)
	, bUseSRGB(*this, true)
{
}


void SMidiNoteContainer::Construct(const FArguments& InArgs)
{
	
	UniqueIDinParent = InArgs._UIDinParent;


	Color.Assign(*this, InArgs._Color);
	AlphaBackgroundBrush.Assign(*this, InArgs._AlphaBackgroundBrush);
	GradientCornerRadius.Assign(*this, InArgs._CornerRadius, FVector4(0.0f));
	ColorBlockSize.Assign(*this, InArgs._Size);

	AlphaDisplayMode.Assign(*this, InArgs._AlphaDisplayMode);
	ColorIsHSV.Assign(*this, InArgs._ColorIsHSV);
	ShowBackgroundForAlpha.Assign(*this, InArgs._ShowBackgroundForAlpha);
	bUseSRGB.Assign(*this, InArgs._UseSRGB);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMidiNoteContainer::SetParentPointer(SPianoRollGraph* inParentGraph)
{
	parentGraph = inParentGraph;
}

void SMidiNoteContainer::SetParentSharedPtr(TSharedPtr<SPianoRollGraph> inParentGraph)
{
	//parentSharedRef = inParentGraph;
	parentPianoRollPanel = inParentGraph;
}

void SMidiNoteContainer::CallOnPressed()
{
}

void SMidiNoteContainer::CallOnVerticalMove(int rowsDelta)
{
}

TEnumAsByte<EButtonClickMethod::Type> SMidiNoteContainer::GetClickMethodFromInputType(const FPointerEvent& MouseEvent) const
{
	if (MouseEvent.IsTouchEvent())
	{
		switch (TouchMethod)
		{
		case EButtonTouchMethod::Down:
			return EButtonClickMethod::MouseDown;
		case EButtonTouchMethod::DownAndUp:
			return EButtonClickMethod::DownAndUp;
		case EButtonTouchMethod::PreciseTap:
			return EButtonClickMethod::PreciseClick;
		}
	}

	return ClickMethod;
}

FReply SMidiNoteContainer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = FReply::Unhandled();
	if (IsEnabled() && (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || MouseEvent.IsTouchEvent()))
	{
		//Press();
		parentPianoRollPanel->isNotePressed = true;
		parentPianoRollPanel->selectedNoteMapIndex = UniqueIDinParent.Get();
		parentPianoRollPanel->dragDeltaAccumulator = 0;
		
		//PressedScreenSpacePosition = MouseEvent.GetScreenSpacePosition();

		EButtonClickMethod::Type InputClickMethod = GetClickMethodFromInputType(MouseEvent);

		if (InputClickMethod == EButtonClickMethod::MouseDown)
		{
			//get the reply from the execute function
			//Reply = ExecuteOnClick();

			//You should ALWAYS handle the OnClicked event.
			ensure(Reply.IsEventHandled() == true);
		}
		else if (InputClickMethod == EButtonClickMethod::PreciseClick)
		{
			// do not capture the pointer for precise taps or clicks
			// 
			Reply = FReply::Handled();
		}
		else
		{
			//we need to capture the mouse for MouseUp events
			Reply = FReply::Handled().CaptureMouse(AsShared());
		}
	}

	//return the constructed reply
	return Reply;
}

FReply SMidiNoteContainer::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	
	//parentSharedRef->isNotePressed = false;
	//parentGraph->isNotePressed = false;
	parentPianoRollPanel->isNotePressed = false;
	parentPianoRollPanel->StopDraggingNote();
	//parentGraph->selectedNoteMapIndex = UniqueIDinParent.Get();
	
	
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SMidiNoteContainer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//UE_LOG(LogTemp, Warning, TEXT("Hello"));
	FVector2D mousePos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	bool resize = false;

	if (mousePos.X < 10)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Left Margin"));
		currentCursor = EMouseCursor::ResizeLeftRight;
		resize = true;
	}


	if (mousePos.X > MyGeometry.GetLocalSize().X - 10)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Right Margin"));
		currentCursor = EMouseCursor::ResizeLeftRight;
		resize = true;
	}

	if(!resize) currentCursor = EMouseCursor::GrabHand;
	
	if (parentPianoRollPanel->isNotePressed)
		{
		parentPianoRollPanel->DragNote(MouseEvent);
		return FReply::Unhandled();
		}
	return FReply::Unhandled();
}

int32 SMidiNoteContainer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const ESlateDrawEffect DrawEffects = bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

	EColorBlockAlphaDisplayMode DisplayMode = AlphaDisplayMode.Get();
	bool bIgnoreAlpha = DisplayMode == EColorBlockAlphaDisplayMode::Ignore;
	FLinearColor InColor = Color.Get();
	if (ColorIsHSV.Get())
	{
		InColor = InColor.HSVToLinearRGB();
	}

	if (ShowBackgroundForAlpha.Get() && InColor.A < 1.0f && !bIgnoreAlpha)
	{
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), AlphaBackgroundBrush.Get(), DrawEffects);
	}

	TArray<FSlateGradientStop> GradientStops;

	switch (DisplayMode)
	{
	default:
	case EColorBlockAlphaDisplayMode::Combined:
		MakeSection(GradientStops, FVector2D::ZeroVector, AllottedGeometry.GetLocalSize(), InColor, InWidgetStyle, false);
		break;
	case EColorBlockAlphaDisplayMode::Separate:
		MakeSection(GradientStops, FVector2D::ZeroVector, AllottedGeometry.GetLocalSize() * 0.5f, InColor, InWidgetStyle, false);
		MakeSection(GradientStops, AllottedGeometry.GetLocalSize() * 0.5f, AllottedGeometry.GetLocalSize(), InColor, InWidgetStyle, true);
		break;
	case EColorBlockAlphaDisplayMode::Ignore:
		MakeSection(GradientStops, FVector2D::ZeroVector, AllottedGeometry.GetLocalSize(), InColor, InWidgetStyle, true);
		break;
	}


	//TODO: this is cool, it works but no reason to do the math on the draw event, we need 
	auto gradRangeIn = TRange<float>(100.0f, 30.0f);
		auto gradRangeOut = TRange<float>(4.0f, 1.0f);
	float cornerRadius =	FMath::GetMappedRangeValueClamped<float>(gradRangeIn, gradRangeOut, AllottedGeometry.GetLocalSize().X);
	FVector4f(cornerRadius, cornerRadius, cornerRadius, cornerRadius);

	FSlateDrawElement::MakeGradient(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(),
		MoveTemp(GradientStops),
		(AllottedGeometry.GetLocalSize().X > AllottedGeometry.GetLocalSize().Y) ? Orient_Vertical : Orient_Horizontal,
		DrawEffects,
		FVector4f(cornerRadius, cornerRadius, cornerRadius, cornerRadius)
	);


	return LayerId + 1;
}

void SMidiNoteContainer::MakeSection(TArray<FSlateGradientStop>& OutGradientStops, FVector2D StartPt, FVector2D EndPt, FLinearColor InColor, const FWidgetStyle& InWidgetStyle, bool bIgnoreAlpha) const
{
	// determine if it is HDR
	const float MaxRGB = FMath::Max3(InColor.R, InColor.G, InColor.B);
	if (MaxRGB > 1.f)
	{
		const float Alpha = bIgnoreAlpha ? 1.0f : InColor.A;
		FLinearColor NormalizedLinearColor = InColor / MaxRGB;
		NormalizedLinearColor.A = Alpha;
		const FLinearColor DrawNormalizedColor = InWidgetStyle.GetColorAndOpacityTint() * NormalizedLinearColor.ToFColor(bUseSRGB.Get());

		FLinearColor ClampedLinearColor = InColor;
		ClampedLinearColor.A = Alpha * MaxRGB;
		const FLinearColor DrawClampedColor = InWidgetStyle.GetColorAndOpacityTint() * ClampedLinearColor.ToFColor(bUseSRGB.Get());

		OutGradientStops.Add(FSlateGradientStop(StartPt, DrawNormalizedColor));
		OutGradientStops.Add(FSlateGradientStop((StartPt + EndPt) * 0.5f, DrawClampedColor));
		OutGradientStops.Add(FSlateGradientStop(EndPt, DrawNormalizedColor));
	}
	else
	{
		FColor DrawColor = InColor.ToFColor(bUseSRGB.Get());
		if (bIgnoreAlpha)
		{
			DrawColor.A = 255;
		}
		OutGradientStops.Add(FSlateGradientStop(StartPt, InWidgetStyle.GetColorAndOpacityTint() * DrawColor));
		OutGradientStops.Add(FSlateGradientStop(EndPt, InWidgetStyle.GetColorAndOpacityTint() * DrawColor));
	}
}

FVector2D SMidiNoteContainer::ComputeDesiredSize(float) const
{
	return ColorBlockSize.Get();
}
