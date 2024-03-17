// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EngravingSubsystem.h"


#include "GlyphTester.generated.h"

/**
 * Abstract widget class to derive blueprint types from. Used to view SMUFL font glyphs and metadata for easier debug of engraving widgets. 
 */
UCLASS(Abstract)
class BKMUSICWIDGETS_API UGlyphTester : public UUserWidget
{
	GENERATED_BODY()

public:

	UGlyphTester(const FObjectInitializer&);
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Font")
	FSlateFontInfo EngravingFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Test")
	int32 NoteHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Test")
	float SpaceTest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Font")
	float FontSize;

	// this is the data asset which describes the SMUFL metadata for each glyph
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Font")
	UMusicFontDataAsset* MusicFontDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Font")
	FSlateFontInfo DebugFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BluePrintSetter = UpdateGlyphString, Category = "Music|Font")
	FName SelectedGlyphName;//= TEXT("—");

	UPROPERTY(BluePrintReadOnly, Category = "Music|Font")
	FVector2D fontMeasureRes;

	UPROPERTY(BluePrintReadOnly, Category = "Music|Font")
	TArray<FName> validCodepoints;

	UPROPERTY(BluePrintReadOnly, Category = "Music|Font")
	TMap<FName, FGlyphCategory> GlyphCategories;

	TCHAR toMeasure;

private:

	UFUNCTION(BluePrintSetter)
	void UpdateGlyphString(FName newString);

	void NativePreConstruct() override;

	int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;


};
