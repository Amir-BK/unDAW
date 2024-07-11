// Fill out your copyright notice in the Description page of Project Settings.
#include "GlyphTester.h"
//#include <SlateCore.h>
//#include <EngravingSubsystem.h>
#include "EngineGlobals.h"
#include "Fonts/FontCache.h"
#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Runtime/Engine/Classes/Engine/RendererSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "Fonts/FontMeasure.h"

UGlyphTester::UGlyphTester(const FObjectInitializer& objectInitializer) :Super(objectInitializer)
{
	//ConstructorHelpers::FClassFinder<UUserWidget> UGlyphTester(TEXT("Glyph Tester Window"));
}

void UGlyphTester::UpdateGlyphString(FName newString)
{
	TSharedRef<FSlateFontMeasure> fontMeasureService = (FSlateApplication::Get().GetRenderer()->GetFontMeasureService());
	SelectedGlyphName = newString;
	//this measures the EM!!!!

	auto FontCache = FSlateApplication::Get().GetRenderer()->GetFontCache();
	auto FontData = FontCache->GetDefaultFontData(EngravingFont);
	FontCache->CanLoadCodepoint(FontData, GEngine->GetEngineSubsystem<UEngravingSubsystem>()->GetUnicodeIntForGlyph(newString));

	FString outString;
	FUnicodeChar::CodepointToString(0xE0A4, outString);

	if (GEngine) {
		toMeasure = TCHAR(GEngine->GetEngineSubsystem<UEngravingSubsystem>()->GetUnicodeIntForGlyph(newString));
	}

	fontMeasureRes = fontMeasureService->Measure(outString, EngravingFont);

	auto WidgetWindow = FSlateApplication::Get().FindWidgetWindow(StaticCastSharedRef<SWindow, SWidget, ESPMode::ThreadSafe>(this->GetCachedWidget().ToSharedRef()));
	FVector2D viewportSize;
	float DPIScale = 1;
	FVector2f WindowSize = FVector2f::ZeroVector;
	if (WidgetWindow != nullptr)
	{
		DPIScale = WidgetWindow->GetNativeWindow()->GetDPIScaleFactor();
		WindowSize = WidgetWindow->GetSizeInScreen();
	}
	//GEngine->GameViewport->GetViewportSize(viewportSize);
	//int32 X = FGenericPlatformMath::FloorToInt(viewportSize.X);
	//int32 Y = FGenericPlatformMath::FloorToInt(viewportSize.Y);

	SpaceTest = DPIScale * EngravingFont.Size;
}

void UGlyphTester::NativePreConstruct()
{
	auto FontCache = FSlateApplication::Get().GetRenderer()->GetFontCache();
	auto FontData = FontCache->GetDefaultFontData(EngravingFont);

	if (GEngine) {
		//SNew(UGlyphTester);
		//this->GetCachedWidget().ToSharedRef();

		auto WidgetWindow = FSlateApplication::Get().FindWidgetWindow(StaticCastSharedRef<SWindow, SWidget, ESPMode::ThreadSafe>(this->GetCachedWidget().ToSharedRef()));
		FVector2D viewportSize;
		float DPIScale = 1;
		if (WidgetWindow != nullptr)
		{
			DPIScale = WidgetWindow->GetNativeWindow()->GetDPIScaleFactor();
		}
		//GEngine->GameViewport->GetViewportSize(viewportSize);
		//int32 X = FGenericPlatformMath::FloorToInt(viewportSize.X);
		//int32 Y = FGenericPlatformMath::FloorToInt(viewportSize.Y);

		SpaceTest = DPIScale;//* EngravingFont.Size;
	}

	for (auto& i : GEngine->GetEngineSubsystem<UEngravingSubsystem>()->GlyphToUnicodeMap)
	{
		if (FontCache->CanLoadCodepoint(FontData, i.Value))
		{
			validCodepoints.Add(i.Key);
		}
	}

	//FontCache->CanLoadCodepoint(FontData, GEngine->GetEngineSubsystem<UEngravingSubsystem>()->GetUnicodeIntForGlyph(newString));
}

int32 UGlyphTester::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	float yOffset = 200.0f;
	FVector2f offsets = FVector2f(0.0f, 0.0f);
	FVector2f offsetsNW = FVector2f(0.0f, 0.0f);
	//float staffSpace = (float) EngravingFont.Size * 72 / 4.0f;
	float staffSpace = (float)fontMeasureRes.Y / 16.0f;
	//UE_LOG(LogTemp, Warning, TEXT("Staff Space, Measurement Based: %f, Font Size Based = %f"), (float)fontMeasureRes.Y / 16.0f, (float)EngravingFont.Size / 4.0f);

	if (IsValid(MusicFontDataAsset) && MusicFontDataAsset->glyphs.Contains(FName(SelectedGlyphName)))
	{
		//MusicFontDataAsset->glyphs[FName(SelectedGlyphName)].metadata.Width;
		if (MusicFontDataAsset->glyphs[FName(SelectedGlyphName)].metadata.anchors.Contains(FName(TEXT("stemUpSE"))))
		{
			//MusicFontDataAsset->glyphs[FName(SelectedGlyphName)].metadata.Width;
			auto point = MusicFontDataAsset->glyphs[FName(SelectedGlyphName)].metadata.anchors[FName(TEXT("stemUpSE"))];
			auto point2 = MusicFontDataAsset->glyphs[FName(SelectedGlyphName)].metadata.anchors[FName(TEXT("stemDownNW"))];
			offsets += point.operator*(FVector2f(staffSpace, -staffSpace));
			offsetsNW += point2.operator*(FVector2f(staffSpace, -staffSpace));
			//offsets.Y += yOffset;
		};
		//offsets.X += (fontMeasureRes.Y / 16) * MusicFontDataAsset->glyphs[FName(SelectedGlyphName)].metadata.Width;
	};
	//MusicFontDataAsset->glyphs.Find(FName(&toMeasure))->metadata

	auto topMargin = FVector2f(0.0f, 500.0f);
	auto drawHeight = FVector2f(0.0f, NoteHeight * (-staffSpace / 2));

	//this is needed so that each glyph is drawn with its center as 0...
	auto ElementRecenterOffset = FVector2f(0.0f, -fontMeasureRes.Y / 2);

	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(1, ElementRecenterOffset.operator+(topMargin).operator+(drawHeight))), // 0 - (staffSpace / 2) * ((glyph->draw_height)) + (topMargin * staffSpace))),
		FString(&toMeasure),
		EngravingFont,
		ESlateDrawEffect::NoPixelSnapping,
		FLinearColor::Black);

	if (!offsets.IsNearlyZero())
	{
		//auto beamSelfOfsset = FVector2f(-0.12f * staffSpace / 2, 0);
		TArray<FVector2f> flagPoints;

		int flagDirection = FMath::Floor((NoteHeight > 3.5) ? -3.5f : 3.5f);
		auto toUsePoint = (NoteHeight > 3.5) ? offsetsNW : offsets;
		auto beamSelfOfsset = (NoteHeight > 3.5) ? FVector2f(0.12f * staffSpace / 2, 0) : FVector2f(-0.12f * staffSpace / 2, 0);

		flagPoints.Add(toUsePoint);
		flagPoints.Add(FVector2f(toUsePoint.X, toUsePoint.Y + flagDirection * (-staffSpace)));

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(1, topMargin.operator+(drawHeight).operator+(beamSelfOfsset))),
			flagPoints,
			ESlateDrawEffect::NoPixelSnapping,
			FLinearColor::Black,
			false,
			0.12f * staffSpace);
	}

	TArray<FVector2f> staffPoints;
	staffPoints.Add(FVector2f(0.0f, 0.0f));
	staffPoints.Add(FVector2f(AllottedGeometry.ToPaintGeometry().GetLocalSize().X, 0.0f));

	for (int i = 0; i < 5; i++)
	{
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(1, topMargin.operator+(FVector2f(0.0f, staffSpace * -i)))),
			staffPoints,
			ESlateDrawEffect::NoPixelSnapping,
			FLinearColor::Black,
			false,
			0.13f * staffSpace);
	}

	return int32();
}