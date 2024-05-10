// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Widgets/Input/SButton.h"
#include <BK_MusicSceneManagerInterface.h>
#include "HarmonixMidi/MusicTimeSpan.h"
#include "Misc/EnumRange.h"
#include "Widgets/Text/STextBlock.h"
//#include <MIDIEditorBase.h>
#include "GlyphButton.generated.h"





DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonClickedEvent, EBKTransportCommands, OutTransportCommand);
/**
 * Simple button that displays glyps from UI Icons font 
 */
UCLASS(Category = "BK Music|Transport|UI")
class BKMUSICWIDGETS_API UTransportGlyphButton : public UWidget
{
	GENERATED_BODY()





	UFUNCTION(BlueprintCallable, Category = "BK Music|Transport")
	bool SetParentEditor(UObject* MidiEditor);

	UFUNCTION(BlueprintCallable, Category = "BK Music|Grid|Transport")
	void SetActiveState(bool isActive);


public:

	static TSharedRef<SButton> CreateTransportButton(EBKTransportCommands Command);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BK Music|Transport")
	TEnumAsByte<EBKTransportCommands> TransportCommand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|Transport")
	FLinearColor IconColor;

	UPROPERTY(BlueprintAssignable, Category = "BK Music|Transport")
	FOnButtonClickedEvent TransportButtonClicked;
	
protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UWidget Interface

	//TSharedRef<SButton> Button;

	bool IsActive = false;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuantizationValueButtonClicked, EMusicTimeSpanOffsetUnits, OutQuantizationValue);
/**
 * Simple button that displays glyps from UI Icons font
 */
UCLASS(Category = "BK Music|Pianoroll|Quantization")
class BKMUSICWIDGETS_API UQuantizationValueGlyphButton : public UWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "BK Music|Grid|Quantization")
	FOnQuantizationValueButtonClicked QuantizationButtonClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|Grid|Quantization|UI")
	FLinearColor IconColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|Grid|Quantization|UI")
	FLinearColor SelectedIconColor = FLinearColor::Yellow;

	UFUNCTION(BlueprintCallable, Category = "BK Music|Grid|Quantization")
	void SetActiveState(bool isActive);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BK Music|Grid|Quantization")
	EMusicTimeSpanOffsetUnits QuantizationValue;

	UPROPERTY(EditAnywhere, Category = "BK Music|Grid|Quantization")
	float FontSize = 24;


protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UWidget Interface

	//TSharedRef<SButton> Button;

	bool IsActive = false;
};
