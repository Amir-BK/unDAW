// Fill out your copyright notice in the Description page of Project Settings.

#include "SEngravingWidget.h"
//#include <SlateCore.h>
#include "EngineGlobals.h"
#include "Fonts/FontMeasure.h"

/*
 int USEngravingWidget::GlypthToHexcode(EGlyphsTypes eGlypthType)
{
	switch (eGlypthType) {
	case Black_Notehead:

		return 0xE0A4;
	case Half_Notehead:
		return 0xE0A3;
	case Whole_Notehead:
		return 0xE0A2;
	case G_Clef:
		return 0xE050;
	case accidentalSharp:
		return 0xE262;
	case F_Clef:
		return 0xE062;
	}

	return 0;
}
*/

int32 USEngravingWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//This shit is to convert unicode to string...

	//Test points
	TArray<FVector2f> StaffPoints;

	//we do this bit to make sure we have the actual font size measured also in designer time but also that we don't measure it every draw frame
	float staffSpace = 12.0f;
	if (bActualFontSizeMeasured)
	{
		staffSpace = staff_spacing;
	}
	else {
	}

	if (bShowGrid)
	{
		for (int i = 0; i < AllottedGeometry.GetLocalSize().Y / (staffSpace / 2); i++)
		{
			StaffPoints.Add(FVector2f(0, (staffSpace / 2) * i));
			StaffPoints.Add(FVector2f(AllottedGeometry.GetLocalSize().X, (staffSpace / 2) * i));

			int gridY = i - 2 * (topMargin + 2);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				StaffPoints,
				ESlateDrawEffect::None,
				FLinearColor::Blue,
				false,
				1.0f);

			FSlateDrawElement::MakeText(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(1, FVector2D(0, (staffSpace / 2) * i))), // 0 - (staffSpace / 2) * ((glyph->draw_height)) + (topMargin * staffSpace))),
				FString::FromInt(gridY),
				DebugFont,
				ESlateDrawEffect::None,
				EngravingColor);

			StaffPoints.Empty();
		}
	}

	for (int i = 0; i < 5; i++)
	{
		StaffPoints.Add(FVector2f(0, (topMargin * staffSpace) + staffSpace * i));
		StaffPoints.Add(FVector2f(AllottedGeometry.GetLocalSize().X, (topMargin * staffSpace) + staffSpace * i));

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			StaffPoints,
			ESlateDrawEffect::None,
			EngravingColor,
			false,
			4.0f);

		StaffPoints.Empty();
	}

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		&BackgroundBrush,
		ESlateDrawEffect::None,
		BackgroundBrush.GetTint(InWidgetStyle));

	//draw the parsed glyphs from the prepared glyph array, we should try to minimize the amount of calculations that happen here,
	//instead store results in the object
	for (auto& glyph : prepared_glyph_objects)
	{
		DrawGlyphElement(glyph, OutDrawElements, LayerId, AllottedGeometry, staffSpace, StaffPoints);
	}
	//draw clef

	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(1, FVector2D(100, topMargin * (clef.clefOffset + staffSpace)))),
		clef.toString(),
		EngravingFont,
		ESlateDrawEffect::None,
		EngravingColor);

	return int32();
}

void USEngravingWidget::prepare_glyphs()
{
	prepared_glyph_objects.Empty();
	auto clefOffset = FBKMusicWidgetsModule::getDiatonicAndAccidental(clef.centerPitch);

	for (auto& event : musical_events)
	{
		if (IsValid(event)) {
			UPreparedGlyph* glyph = NewObject<UPreparedGlyph>(this);
			glyph->draw_horizontal = event->start_time;

			UPitchedMusicalEvent* asPitchedEvent = Cast<UPitchedMusicalEvent>(event);
			if (IsValid(asPitchedEvent)) {
				int tempPitch = asPitchedEvent->pitch;
				auto notestruct = FBKMusicWidgetsModule::getDiatonicAndAccidental(asPitchedEvent->pitch);
				glyph->draw_height = (notestruct.X) + (7 * (int)(notestruct.Z)) - (clefOffset.X + 2 + (7 * clefOffset.Z));

				//(notestruct.X) + (7 * (int)(asPitchedEvent->pitch / 12))- (clefOffset + (7 * clefOffsetOctave));
				glyph->mainGlyph = EGlyphsTypes::Black_Notehead;
				glyph->accidentalGlyph = notestruct.Y == 1 ? EGlyphsTypes::accidentalSharp : EGlyphsTypes::no_glyph;
				glyph->drawStem = true;
				glyph->prepareStringRepresentation();
			}
			else {
				glyph->mainGlyph = EGlyphsTypes::Rest_Quarter;
				glyph->draw_height = 0;
				glyph->accidentalGlyph = EGlyphsTypes::no_glyph;
			}

			prepared_glyph_objects.Push(glyph);
		}
	}
}

int USEngravingWidget::ConvertDrawHeightToPitch(int inDrawHeight)
{
	int basePitch = 71; //this is our center line, in g clef... we'll get there.

	TArray<int32> scaleDegrees;
	scaleDegrees = { 0, 1, 3, 5, 6, 8, 10 };
	int drawOctave = 12 * ((int)((-inDrawHeight) / 7));
	if (inDrawHeight > 0) {
		drawOctave -= 12;
	}

	auto wrapResult = FBKMusicWidgetsModule::wrapAroundArray(scaleDegrees, -inDrawHeight);

	//UE_PRIVATE_LOG(PREPROCESSOR_NOTHING, constexpr, LogTemp, Warning, u"Pitch: %d",basePitch + wrapResult.X);
	//UE_PRIVATE_LOG(PREPROCESSOR_NOTHING, constexpr, LogTemp, Warning, u"Octave int: %d",12 * wrapResult.Y);

	//return basePitch + scaleDegrees[(int)abs(inDrawHeight) % 7] + drawOctave;
	return basePitch + wrapResult.X + 12 * wrapResult.Y;
}

FVector2D USEngravingWidget::getGridLocationAtScreenPosition(FVector2f inPosition)
{
	bool snap = true;

	return FVector2D((int)inPosition.X, (int)inPosition.Y / gridSpacing - 2 * (topMargin + 2));
}

void USEngravingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void USEngravingWidget::NativePreConstruct()
{
	clef = FClefStruct(E_clef);

	//Measure font size and staff spacing, once measured set the bool to true so we don't check it every frame.
	//TCHAR unichar = TCHAR();
	const TSharedRef<FSlateFontMeasure> FontMeasureService = (FSlateApplication::Get().GetRenderer()->
		GetFontMeasureService());

	//this measures the EM!!!!
	measuredY = FontMeasureService->Measure(FString(1, TEXT("—")), EngravingFont).X;
	staff_spacing = measuredY / 4;
	bActualFontSizeMeasured = true;
	for (auto& event : musical_events)
	{
		UPitchedMusicalEvent* asPitchedEvent = Cast<UPitchedMusicalEvent>(event);
		if (IsValid(asPitchedEvent)) {
			asPitchedEvent->UpdatePitch(asPitchedEvent->pitch);
		};
	}

	PrepareGrid();
	prepare_glyphs();
}

void USEngravingWidget::PrepareGrid()
{
	gridSpacing = staff_spacing;
	gridMax = GetCachedGeometry().GetLocalSize().Y / gridSpacing - 2 * (topMargin + 2);
	gridMin = -2 * (topMargin + 2);
}

UMusicalEvent* USEngravingWidget::AddMusicalEvent(UMusicalEvent* newEvent)
{
	return nullptr;
}

UMusicalEvent* USEngravingWidget::CreateAndAddMusicalEvent(TEnumAsByte<EMusicalEventTypes> eventType, FQuartzQuantizationBoundary start_boundry, FQuantizedDuration event_duration, int32 pitch)
{
	return nullptr;
}

void USEngravingWidget::DrawGlyphElement(UPreparedGlyph* const& glyph, FSlateWindowElementList& OutDrawElements, const int32& LayerId, const FGeometry& AllottedGeometry, float staffSpace, TArray<FVector2f>& StaffPoints) const
{
	const FLinearColor GlyphColor = glyph->bGlyphSelected ? FLinearColor::Blue : EngravingColor;
	FString MainGlyph;

	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(1, FVector2D(glyph->draw_horizontal, 4 * staffSpace))),//(topMargin * staffSpace) - (glyph->draw_height) * (staffSpace / 2)))), // 0 - (staffSpace / 2) * ((glyph->draw_height)) + (topMargin * staffSpace))),
		MainGlyph.AppendChar(static_cast<TCHAR>(GEngine->GetEngineSubsystem<UEngravingSubsystem>()->GetUnicodeIntForGlyph(
			FName(TEXT("noteheadBlack"))))),
		EngravingFont,
		ESlateDrawEffect::None,
		GlyphColor);

	if (glyph->accidentalGlyph != EGlyphsTypes::no_glyph) {
		FString accidentalGlyph;

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(1, FVector2D(glyph->draw_horizontal - staffSpace, (topMargin * staffSpace) - (12 + glyph->draw_height) * (staffSpace / 2)))), // 0 - (staffSpace / 2) * ((glyph->draw_height)) + (topMargin * staffSpace))),
			accidentalGlyph.AppendChar(GlyphEnumToChar(glyph->accidentalGlyph)),
			EngravingFont,
			ESlateDrawEffect::None,
			GlyphColor);
	}

	// stems

	if (glyph->drawStem)
	{
		float cutOutNWX = 1.18;
		float cutOutNWY = -0.168;
		StaffPoints.Add(FVector2f((cutOutNWX - 0.12) * staffSpace, cutOutNWY * staffSpace / 2));
		StaffPoints.Add(FVector2f((cutOutNWX - 0.12) * staffSpace, (cutOutNWY - 6) * staffSpace / 2));

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(1, FVector2D(glyph->draw_horizontal, 1 * ((2 * topMargin - glyph->draw_height + 4) * (staffSpace / 2))))),
			StaffPoints,
			ESlateDrawEffect::NoPixelSnapping,
			GlyphColor,
			false,
			staffSpace * 0.24);

		StaffPoints.Empty();
	}

	//needs serious cleanup! this is the function that draws the extra staff lines
	if (FMath::Abs(glyph->draw_height) >= 6)
	{
		const int Base = FMath::Sign(glyph->draw_height) > 0 ? 0 : 4;
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Draw Height %d!"), glyph->draw_height));
		for (int i = 1; i <= FMath::Abs(glyph->draw_height / 2) - 2; i++) {
			const float PointY = (topMargin * staffSpace) - FMath::Sign(glyph->draw_height) * (staffSpace * (i + (Base)));
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Point Y %f!"), pointY));
			StaffPoints.Add(FVector2f(glyph->draw_horizontal - staffSpace, PointY));
			StaffPoints.Add(FVector2f(glyph->draw_horizontal + 2 * staffSpace, PointY));

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				StaffPoints,
				ESlateDrawEffect::None,
				EngravingColor,
				false,
				4.0f);

			StaffPoints.Empty();
		}
	}
}

void USEngravingWidget::NativeOnInitialized() {
	/*
	clef = FClefStruct(E_clef);

	//Measure font size and staff spacing, once measured set the bool to true so we don't check it every frame.
	TCHAR unichar = TCHAR(FBKMusicWidgetsModule::GlyphToHexTest(EGlyphsTypes::Black_Notehead));
	TSharedRef<FSlateFontMeasure> fontMeasureService = (FSlateApplication::Get().GetRenderer()->GetFontMeasureService());

	measuredY = fontMeasureService->Measure(FString(1, &unichar), EngravingFont).Y;
	staff_spacing = measuredY / 16;
	bActualFontSizeMeasured = true;

	//prepare_glyphs();
	*/
}

TCHAR GlyphEnumToChar(EGlyphsTypes inGlyph) {
	return TCHAR(FBKMusicWidgetsModule::GlyphToHexTest(inGlyph));
}

void UPitchedMusicalEvent::UpdatePitch(int newPitch)
{
	pitch = newPitch;
	auto result = FBKMusicWidgetsModule::getDiatonicAndAccidental(newPitch);
	FString returnString;

	switch (result.X) {
	case 0:
		returnString = "C";
		break;

	case 1:
		returnString = "D";
		break;
	case 2:
		returnString = "E";
		break;
	case 3:
		returnString = "F";
		break;
	case 4:
		returnString = "G";
		break;
	case 5:
		returnString = "A";
		break;
	case 6:
		returnString = "B";
		break;
	default:
		returnString = "POOP";
		break;
	}

	if (result.Y == 1)
	{
		returnString.Append("#");
	}

	returnString.Append(FString::FromInt(result.Z));

	NoteName = returnString;
}