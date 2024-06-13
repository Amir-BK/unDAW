// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
//#include <SEngravingWidget.h>
#include "EngravingSubsystem.h"
#include "Framework/Application/SlateApplication.h"
//#include "SimpleEngraving.generated.h"

//#include <SEngravingWidget.h>

struct FMeasuredGlyph
{
	TCHAR glyph;
	float measuredX;
	float measuredY;
};

class FBKMusicWidgetsModule : public IModuleInterface
{
public:
	UPROPERTY(BlueprintReadOnly)
	FString PluginContentDir;

	//UPROPERTY(BlueprintReadOnly)
	static TMap<FName, FMusicGlyph> GlyphMap;

	static constexpr int GlyphToHexTest(const EGlyphsTypes glyph)
	{
		switch (glyph) {
		case Black_Notehead:

			return 0; // FBKMusicWidgetsModule::getGlyphMap().Find(FName(TEXT("noteheadBlack")))->glyphCode;
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
		case Rest_Quarter:
			return 0xE4E5;
		}

		return 0;
	}

	static TMap<FName, FMusicGlyph> getGlyphMap() {
		return FBKMusicWidgetsModule::GlyphMap;
	}

	static FMeasuredGlyph GetMeasuredGlyphFromHex(const int& codepoint);

	//FString PluginDir;

	// the X of the vector is the degree of the scale, the Y is the accidental, the Z is the octave
	static FIntVector getDiatonicAndAccidental(int inNote)
	{
		TArray<int32> scaleDegrees = { 0, 2, 4, 5, 7, 9, 11 };
		auto wraparound = wrapAroundArray(scaleDegrees, inNote);

		if (scaleDegrees.Contains(inNote % 12))
		{
			return FIntVector(scaleDegrees.Find(inNote % 12), 0, (int)(inNote / 12));
		}
		else {
			return FIntVector(scaleDegrees.Find(((inNote + 11) % 12)), 1, (int)(inNote / 12));
		}
	}

	static const FIntVector2 wrapAroundArray(const TArray<int32> inArray, const int inIndex)
	{
		int mod_in = inIndex % inArray.Num();
		int wrap = FMath::Floor((float)inIndex / (float)inArray.Num());

		if (mod_in >= 0)
		{
			return FIntVector2(inArray[mod_in], wrap);
		}
		else {
			return FIntVector2(inArray[inArray.Num() + mod_in], wrap);
		}
	}

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
