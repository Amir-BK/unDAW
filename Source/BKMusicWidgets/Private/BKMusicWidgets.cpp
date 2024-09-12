// Copyright Epic Games, Inc. All Rights Reserved.

#include "BKMusicWidgets.h"
#include "UObject/UObjectArray.h"
//#include "Serialization/JsonSerializer.h"
//#include <EngineFontServices.h>
#include "Fonts/FontCache.h"
#include "Fonts/CompositeFont.h"
#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Runtime/Engine/Classes/Engine/RendererSettings.h"
#include "UnDAWStyle.h"
#include "Fonts/FontMeasure.h"
//#include <EngineFontServices.h>

#define LOCTEXT_NAMESPACE "FBKMusicWidgetsModule"

FMeasuredGlyph FBKMusicWidgetsModule::GetMeasuredGlyphFromHex(const int& codepoint)
{
	FMeasuredGlyph returnGlyph = FMeasuredGlyph();
	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("unDAW"))->GetBaseDir();
	TSharedRef<FSlateFontMeasure> fontMeasureService = (FSlateApplication::Get().GetRenderer()->GetFontMeasureService());
	//SelectedGlyphName = newString;
	//this measures the EM!!!!

	auto FontCache = FSlateApplication::Get().GetRenderer()->GetFontCache();
	auto FontData = FontCache->GetDefaultFontData(FSlateFontInfo(PluginDir / TEXT("Resources/UtilityIconsFonts/icons.ttf"), 24));
	FontCache->CanLoadCodepoint(FontData, codepoint);

	FString outString;
	FUnicodeChar::CodepointToString(codepoint, outString);

	FVector2f fontMeasureRes = fontMeasureService->Measure(outString, FSlateFontInfo(PluginDir / TEXT("Resources/UtilityIconsFonts/icons.ttf"), 24));

	returnGlyph.measuredX = fontMeasureRes.X;
	returnGlyph.measuredY = fontMeasureRes.Y;
	returnGlyph.glyph = TCHAR(codepoint);

	return returnGlyph;
}

void FBKMusicWidgetsModule::StartupModule()
{
	//GlyphsJSON.Get()->TryGetField(TEXT("noteheadBlack")).Get()->AsObject()->TryGetField(TEXT("codepoint")).Get()->AsString();
	FUndawStyle::Get();
}

void FBKMusicWidgetsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBKMusicWidgetsModule, BKMusicWidgets)