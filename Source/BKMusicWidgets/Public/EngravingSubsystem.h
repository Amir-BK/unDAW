// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Dom/JsonValue.h"
#include "Engine/Engine.h"
#include "Sound/QuartzQuantizationUtilities.h"
#include "Misc/FileHelper.h"
#include "Fonts/FontCache.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Framework/Application/SlateApplication.h"

//#include "Subsystems/Subsystem.h"
#include "Subsystems/EngineSubsystem.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Engine/Font.h"

#include "EngravingSubsystem.generated.h"





UENUM(BlueprintType)
enum EClefs
{
	gClef,
	fClef
};

UENUM(BlueprintType)
enum EMusicalEventTypes
{
	Pitched_Event,
	Rest,
	Key_Change,
	Tempo_Change,
	Time_Mesaure_Change
};


USTRUCT(BlueprintType)
struct FQuantizedDuration
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//TEnumAsByte<EQuartzCommandQuantization> duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|Engraving|Tests")
	int32 multiplier = 1;

};

//In the future we'll want to read the glyphs from the JSON
UENUM(BlueprintType)
enum EGlyphsTypes
{
	none,
	Black_Notehead,
	Half_Notehead,
	Whole_Notehead,
	G_Clef,
	F_Clef,
	accidentalFlat,
	accidentalSharp,
	Rest_Quarter

};

// represents the conversion from chromatic key to diatonic key
USTRUCT(BlueprintType)
struct FMusicGlyph
{
	GENERATED_USTRUCT_BODY()

	//value is in hex
	int glyphCode;

	FName glyphName;

	FMusicGlyph() {
	};
};


// what we read from the font metadata json files, this needs to be populated for the specific font we load
USTRUCT(BlueprintType)
struct FGlyphMetaData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "BK Music|Engraving|Tests")
	float Width = 1.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "BK Music|Engraving|Tests")
	TMap<FName, FVector2f> anchors;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "BK Music|Engraving|Tests")
	TMap<FName, FVector2f> bbox;

};

// this is the codepoint for the glyph read from the glyphnames JSON, it is generic for all SMUFL fonts and it is not guranteed that our loaded font will contain the glyph for the codepoint, we need to check that with the renderer.
USTRUCT(BlueprintType)
struct FGlyphData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "BK Music|Engraving|Tests")
	FName GlyphName;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "BK Music|Engraving|Tests")
	int32 codepoint;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "BK Music|Engraving|Tests")
	FGlyphMetaData metadata;

	FGlyphData() {};

	FGlyphData(FName inglyphName, int32 incodepoint)
	{
		GlyphName = inglyphName;
		codepoint = incodepoint;
	}


};

// the categories of glyphs taken from SMUFL classes.JSON, used for organizing the glyphs.
USTRUCT(BlueprintType)
struct FGlyphCategory
{
	GENERATED_BODY()

	UPROPERTY(BluePrintReadOnly, Category = "BK Music|Engraving|Tests")
	TArray<FName> ContentGlyphs;
};



UCLASS(BlueprintType)
class UMusicFont : public UObject
{
	GENERATED_BODY()

public:

	bool initialized = false;

	UPROPERTY(BlueprintReadOnly, Category = "BK Music|Engraving|Tests")
	TMap<FName, FGlyphData> glyphs;

	UPROPERTY(BlueprintReadOnly, Category = "BK Music|Engraving|Tests")
	FSlateFontInfo Font;

	UPROPERTY(BlueprintReadOnly, DisplayName = "Staff Space", Category = "BK Music|Engraving|Tests")
	float staff_Space = 0;
};



//we shall comandeer this class to save some stuff. 
UCLASS()
class BKMUSICWIDGETS_API UMusicFontDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:

	// for some reason SMUFL fonts are inconsistent when measured on their vertical axis, Bravura is twice as tall as Petaluma, so for petaluma put 1, bravura 2;
	UPROPERTY(EditAnywhere, DisplayName = "Measurement Scalar", Category = "BK Music|Engraving|Tests")
	float measurementMultiplier;

	/** The font object (valid when used from UMG or a Slate widget style asset) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowedClasses = "/Script/Engine.Font", DisplayName = "Font"), Category = "BK Music|Engraving|Tests")
	TObjectPtr<const UObject> Font;

	//the map of the glyphs populated with their relevant metadata for the font family. 
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "BK Music|Engraving|Tests")
	TMap<FName, FGlyphData> glyphs;


};


UCLASS(BlueprintType, EditInlineNew, Meta = (UsesHierarchy = "true", ShowInnerProperties = "true"), CollapseCategories, Category = "BK Music|Engraving|Tests")
class BKMUSICWIDGETS_API UMusicalEvent : public UObject
{

	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|Engraving|Tests")
	int32 duration = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|Engraving|Tests")
	float velocity = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|Engraving|Tests")
	int32 start_time = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|Engraving|Tests")
	FQuantizedDuration QuartzDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|Engraving|Tests")
	FQuartzQuantizationBoundary QuartzStarttime;

	//this is the event which is 'note off' for this guy, 
	UPROPERTY()
	UPitchedMusicalEvent* LinkedEvent;

};

UCLASS(BlueprintType, Meta = (UsesHierarchy = "true", ShowInnerProperties = "true"), CollapseCategories, Category = "BK Music|Engraving|Tests")
class BKMUSICWIDGETS_API UPitchedMusicalEvent : public UMusicalEvent
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BluePrintSetter = UpdatePitch, meta = (ExposeOnSpawn = "true", EditInLine = "true", ClampMin = "0", ClampMax = "127"), Category = "BK Music|Engraving|Tests")
	int pitch = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BK Music|Engraving|Tests")
	FString NoteName = "G6";

	//used for previewing musical events before the user adds them
	bool bIsPreview = false;


	UFUNCTION(BluePrintSetter, Category = "BK Music|Engraving|Tests")
	void UpdatePitch(int newPitch);

};

UCLASS(BlueprintType, EditInlineNew, Meta = (UsesHierarchy = "true", ShowInnerProperties = "true"), CollapseCategories, Category = "BK Music|Engraving|Tests")
class BKMUSICWIDGETS_API UMusicSceneTrack : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite,  meta = (ShowInnerProperties = "true", DisplayPriority = "1"), Category = "BK Music|Engraving|Tests")
	FString TrackName;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced,  meta = (ShowInnerProperties = "true", DisplayPriority = "1"), Category = "BK Music|Engraving|Tests")
	TArray<UMusicalEvent*> contentEvents;


};

UCLASS(BlueprintType)
class BKMUSICWIDGETS_API UMusicSceneAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, meta = (ShowInnerProperties = "true", DisplayPriority = "1"), Category = "BK Music|Engraving|Tests")
	TArray<UMusicalEvent*> contentTest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, meta = (ShowInnerProperties = "true", DisplayPriority = "1"), Category = "BK Music|Engraving|Tests")
	TArray<UMusicSceneTrack*> Tracks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|Engraving|Tests")
	float startingBPM;
};

/**
 * Engraving subsystem, loads SMUFL fonts, manages their metadata, stuff. 
 */
UCLASS()
class BKMUSICWIDGETS_API UEngravingSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	//virtual void Deinitialize() override;
	// End USubsystem

	UFUNCTION(BlueprintCallable, Category = "BK Music|Engraving|Tests")
	int32 GetUnicodeIntForGlyph(FName GlyphName);

	UFUNCTION(BlueprintCallable, Category = "BK Music|Engraving|Tests")
	void PopulateDataAssetWithGlyphDataFromJSON(UFont* FontObject, FFilePath Metadata_File_Path, UMusicFontDataAsset* Font_Data_Asset);

	

	UPROPERTY(BluePrintReadOnly, Category = "BK Music|Engraving|Tests")
	TMap<FName, int32> GlyphToUnicodeMap;

	UPROPERTY(BluePrintReadOnly, Category = "BK Music|Engraving|Tests")
	TMap<FName, FGlyphCategory> SMUFLClasses;

	UPROPERTY(BluePrintReadOnly, DisplayName = "Engraving Fonts", Category = "BK Music|Engraving|Tests")
	TMap<FName, UMusicFont*> music_fonts;


	
	 constexpr static bool IsNoteInCmajor(int const noteIn) {
		
		if(noteIn % 12 == 0) return true;
		if(noteIn % 12 == 2) return true;
		if(noteIn % 12 == 4) return true;
		if(noteIn % 12 == 5) return true;
		if(noteIn % 12 == 7) return true;
		if(noteIn % 12 == 9) return true;
		if(noteIn % 12 == 11) return true;
	
		
		return false;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BK Music|Engraving|Tests")
	static bool isNoteInCDiatonic(int const note) {
	
		return IsNoteInCmajor(note);
	};
	
	
	
static char *combineStrings(char* inputA, char* inputB) {
		size_t len = 0, lenB = 0;
		while(inputA[len] != '\0') len++;
		while(inputB[lenB] != '\0') lenB++;
		char* output = (char*) malloc(len+lenB);
		printf((char*)output,"%s%s",inputA,inputB);
		return output;
	}
	
	
   //TODO: figure out how to also return the octave here, otherwise kinda silly
	
#define NOTESYMBOL(c) return FString(TEXT(c))
	
	static FString const pitchNumToStringRepresentation(int const noteIn)
	{
		int mod12 = noteIn % 12;
		//char* octave = TCHAR::From noteIn / 12;

		switch (mod12) {
		case 0:
				NOTESYMBOL("C");
			break;

		case 1:
			   NOTESYMBOL("C#");
			break;
		case 2:
			NOTESYMBOL("D");
			break;
		case 3:
			NOTESYMBOL("D#");
			break;
		case 4:
				NOTESYMBOL("E");
			break;
		case 5:
				NOTESYMBOL("F");
			break;
		case 6:
				NOTESYMBOL("F#");
			break;
		case 7:
				NOTESYMBOL("G");
			break;
		case 8:
				NOTESYMBOL("G#");
			break;
		case 9:
				NOTESYMBOL("A");
			break;
		case 10:
				NOTESYMBOL("A#");
			break;
		case 11:
				NOTESYMBOL("B");
			break;

				
				

		}
		
		//returnString +=
		
		
		return TEXT("Not Great!");
		//return &retValue[0];
		 
	}
	
};
