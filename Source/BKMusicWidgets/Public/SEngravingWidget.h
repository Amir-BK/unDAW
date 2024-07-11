// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/IPluginManager.h"
//#include "Json.h"
#include "Dom/JsonValue.h"
#include "BKMusicWidgets.h"
#include "EngravingSubsystem.h"
//#include <MidiObjects/MidiAsset.h>
#include "EngineGlobals.h"
#include "SEngravingWidget.generated.h"

TCHAR GlyphEnumToChar(EGlyphsTypes inGlyph);

USTRUCT(BlueprintType)
struct FClefStruct
{
	GENERATED_BODY()

	TCHAR clefGlyphString;
	int clefOffset;
	// this pitch will be rendered on the center of the clef, so 67 in gClef - actually should be 71!! the b in the middle
	int centerPitch;

	FString toString() const {
		FString str;
		return str.AppendChar(clefGlyphString);
	}

	FClefStruct()
	{
		clefGlyphString = GlyphEnumToChar(EGlyphsTypes::G_Clef);
		clefOffset = 10;
		centerPitch = 67;
	};

	FClefStruct(EClefs clef)
	{
		switch (clef)
		{
		case EClefs::gClef:
			clefGlyphString = GlyphEnumToChar(EGlyphsTypes::G_Clef);
			clefOffset = 10;
			centerPitch = 67;
			break;
		case EClefs::fClef:
			clefGlyphString = GlyphEnumToChar(EGlyphsTypes::F_Clef);
			clefOffset = 14;
			centerPitch = 35;
			break;
		}
	}
};

// represents a processed glyph, values should be in staff spaces.

UCLASS(BlueprintType)
class UPreparedGlyph : public UObject
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetMainGlyph, meta = (ExposeOnSpawn = "true"), Category = "BK Music|Engraving|Tests")
	TEnumAsByte<EGlyphsTypes> mainGlyph = EGlyphsTypes::Black_Notehead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetAccidentalGlyph, meta = (ExposeOnSpawn = "true"), Category = "BK Music|Engraving|Tests")
	TEnumAsByte<EGlyphsTypes> accidentalGlyph = EGlyphsTypes::no_glyph;

	// vertical height to draw the note, in staff spaces ((in half staff spaces?)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "BK Music|Engraving|Tests")
	int draw_height = 0;

	//horizontal placing, will also be used for draw culling probably, in draw units for now
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "BK Music|Engraving|Tests")
	int draw_horizontal = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BK Music|Engraving|Tests")
	FString builtString = "";

	UPROPERTY(BluePrintReadOnly, Category = "BK Music|Engraving|Tests")
	bool bGlyphSelected = false;

	UPROPERTY(BluePrintReadOnly, Category = "BK Music|Engraving|Tests")
	bool drawStem = false;

	UPreparedGlyph() {
		prepareStringRepresentation();
	}

	//string representation for the glyph... it really might be preferable to cache these somewhere...
	void prepareStringRepresentation()
	{
		if (GEngine)
		{
			builtString.Empty();
			TCHAR unichar = TCHAR(GEngine->GetEngineSubsystem<UEngravingSubsystem>()->GetUnicodeIntForGlyph(FName(TEXT("noteheadBlack"))));
			TCHAR accidental = TCHAR(GEngine->GetEngineSubsystem<UEngravingSubsystem>()->GetUnicodeIntForGlyph(FName(TEXT("accidentalSharp"))));
			builtString.AppendChar(accidental);
			builtString.AppendChar(unichar);
		}
	}

	UFUNCTION(BlueprintSetter)
	void SetMainGlyph(TEnumAsByte<EGlyphsTypes> glyph) {
		mainGlyph = glyph;
		prepareStringRepresentation();
	}

	UFUNCTION(BlueprintSetter)
	void SetAccidentalGlyph(TEnumAsByte<EGlyphsTypes> glyph) {
		accidentalGlyph = glyph;
		prepareStringRepresentation();
	}
};

/**
 *
 */
UCLASS(Abstract)
class BKMUSICWIDGETS_API USEngravingWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MIDI test")
	//UMidiAsset* MidiFile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FSlateBrush BackgroundBrush;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance")
	FSlateFontInfo EngravingFont;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance")
	FSlateFontInfo DebugFont;

	//The margin from the top of the canvas, in staff spaces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	int topMargin;

	UPROPERTY(BlueprintReadOnly, Category = "Music|Grid")
	float gridSpacing;

	UPROPERTY(BlueprintReadOnly, Category = "Music|Grid")
	int gridMin;

	UPROPERTY(BlueprintReadOnly, Category = "Music|Grid")
	int gridMax;

	UPROPERTY(BlueprintReadOnly, Category = "Music|Grid")
	float measuredY;

	UPROPERTY(BlueprintReadOnly, Category = "Music|Grid")
	float measuredX;

	UFUNCTION(BluePrintCallable, Category = "Music|Grid")
	FVector2D getGridLocationAtScreenPosition(FVector2f inPosition);

	UFUNCTION(BluePrintCallable, Category = "Music|Grid")
	int ConvertDrawHeightToPitch(int inDrawHeight);

	// this is the data asset which describes the SMUFL metadata for each glyph
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Music|Font")
	UMusicFontDataAsset* MusicFontDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor EngravingColor = FLinearColor::Black;

	//the amount of pixels calculated given the font size, this value is only reliable in runtime as the font actually has to be measured
	UPROPERTY(BlueprintReadOnly, Category = "Appearance")
	float staff_spacing;

	//add musical events to this array
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BluePrintSetter = UpdateArray, Instanced, Category = "Music|Events", meta = (ShowInnerProperties = "true", DisplayPriority = "1"))
	TArray<UMusicalEvent*> musical_events;

	UPROPERTY(EditAnywhere, BlueprintReadwrite, Category = "Music|Grid")
	bool bShowGrid = true;

	UPROPERTY(EditAnywhere, BlueprintReadwrite, Category = "Music|Fonts")
	float fFactorTest = 0.84f;

	UFUNCTION(BluePrintSetter)
	void UpdateArray(TArray<UMusicalEvent*> newEvents) {
		musical_events = newEvents;
		prepare_glyphs();
	}

	UPROPERTY(BlueprintReadWrite, Category = "Music")
	TArray<UPreparedGlyph*> prepared_glyph_objects;

	UFUNCTION(BlueprintCallable, Category = "BK Music|Engraving|Tests")
	void prepare_glyphs();

	// add a previously constructed musical event
	UFUNCTION(BlueprintCallable, Category = "Music|Events")
	UMusicalEvent* AddMusicalEvent(UPARAM(ref) UMusicalEvent* newEvent);

	// constructs a new musical event of the relevant type and adds it to the musical events array.
	UFUNCTION(BlueprintCallable, Category = "Music|Events")
	UMusicalEvent* CreateAndAddMusicalEvent(TEnumAsByte<EMusicalEventTypes> eventType, FQuartzQuantizationBoundary start_boundry, FQuantizedDuration event_duration, int32 pitch);

	bool bActualFontSizeMeasured = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Defaults")
	TEnumAsByte<EClefs> E_clef;
	//EClefs clef = EClefs::gClef;

	FClefStruct clef;

	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);

private:

	void DrawGlyphElement(UPreparedGlyph* const& glyph, FSlateWindowElementList& OutDrawElements, const int32& LayerId, const FGeometry& AllottedGeometry, float staffSpace, TArray<FVector2f>& StaffPoints) const;

	int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	void NativeOnInitialized() override;

	void NativePreConstruct() override;

	void PrepareGrid();
};
