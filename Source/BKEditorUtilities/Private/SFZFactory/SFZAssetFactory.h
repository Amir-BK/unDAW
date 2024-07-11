// Fill out your copyright notice in the Description page of Project Settings.

// really all the logic for creating a fusion patch from an SFZ asset is contained in the factory, the FK SFZ asset and regions are really just
// temporary data containers at this point.

#pragma once

#include "CoreMinimal.h"
#include "SFZ/UnDAWSFZAsset.h"
#include "Factories/Factory.h"
#include "HarmonixDsp/FusionSampler/FusionPatch.h"
#include "SlateFwd.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "SFZAssetFactory.generated.h"

class USFZAssetFactory;

//the actual settings payload, this is used by the factory when creating the fusion patch
USTRUCT(Category = "BK Music|SFZ|Import Settings")
struct FSFZImportSettings {
	GENERATED_BODY()

	bool bUnpitched = false;
	bool velToGain = true;
	float ampegAttack = 0.0f;
	float ampegRelease = 0.03f;
	float ampegDecay = 0.0f;
	float ampegHold = 0.0f;

	EKeyzoneSelectMode keyzoneSelectMode = EKeyzoneSelectMode::Layers;
};

// widget for the settings dialog
class SImportSFZSettingsDialog : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SImportSFZSettingsDialog)
		{}

	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs);

	FSFZImportSettings settings = FSFZImportSettings();

	const FTextBlockStyle SFZImportInfoStyle = FTextBlockStyle().SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 10)).SetColorAndOpacity(FLinearColor::White);
	TSharedPtr<STextBlock> SFZFileInfoText;

	TSharedPtr<SButton> OkButton;
	TSharedPtr<SCheckBox> unpitchedCheckbox;
	TSharedPtr<SCheckBox> velToGainCheckbox;

	//TSharedPtr<SNumericEntryBox<float>> AmpegTest;

	int32 KeyzoneModeGetCurrentValue() const
	{
		return static_cast<int32>(settings.keyzoneSelectMode);
	}

	void SetKeyzoneMode(int32 InEnumValue, ESelectInfo::Type)
	{
		settings.keyzoneSelectMode = static_cast<EKeyzoneSelectMode>(InEnumValue);
	}

	void setAmpegHold(float value)
	{
		settings.ampegHold = value;
	}

	void setAmpegRelease(float value)
	{
		settings.ampegRelease = value;
	};

	void setAmpegAttack(float value)
	{
		settings.ampegAttack = value;
	};

	void setAmpegDecay(float value)
	{
		settings.ampegDecay = value;
	}

	TOptional<float> getAmpegHold() const
	{
		return settings.ampegHold;
	}

	TOptional<float> getAmpegRelease() const
	{
		return settings.ampegRelease;
	};

	TOptional<float> getAmpegAttack() const
	{
		return settings.ampegAttack;
	};

	TOptional<float> getAmpegDecay() const
	{
		return settings.ampegDecay;
	}

	void setUnpitchedBool(ECheckBoxState NewState)
	{
		settings.bUnpitched = NewState == ECheckBoxState::Checked;
	}

	void setVelToGain(ECheckBoxState NewState)
	{
		settings.velToGain = NewState == ECheckBoxState::Checked;
	}
};

//@brief helper class to traverse a region and associate opcodes to values through a slow recursive algorithm that seems to work rather robustly given the lack of strict white spacing of sfz files
UCLASS(Category = "BK Music|SFZ|Internal Classes")
class USFZOpCodeLineParser : public UObject
{
	GENERATED_BODY()

	FString TraversalString;
	int CurrentIndex;
public:
	TMap<FName, FString> OpCodeToValuesMap;
	TMap<FString, FString> DefinesMap;

	// any region should be agnostic to the level of the hierarchy it's at and in fact the methods already implement it as such
	UPROPERTY()
	USFZOpCodeLineParser* ParentPtr = nullptr;

	/**
	*@brief this is the main caller function of this class, give it any string of any length and it will break it into opcode key/token values, also substitutes SFZ defines that were declared earlier in the file
	* @param StringToTraverse the string to be recursively broken into op codes.
	*/
	bool TraverseString(FString StringToTraverse);

	void CombineOpCodesWithParents();
	void PopulateSFZRegion(USFZRegion* Region, UFKSFZAsset* NewAsset, USFZAssetFactory* Factory);

private:
	//the private implementation of the recursive function implementing end conditions
	FString CheckForOpCodesAndTraverse(const FString& SubString);

	// the only 'strict' requirement in SFZ is that opcode tokens are delimited by an '=' sign and that the 'key' of the opcode extends until the first whitespace before the delimiter, therefore, we must know the index of the next delimeter
	int GetNextOpCodeStartIndex(int CurrentOpCode);
};

UCLASS()
class BK_EDITORUTILITIES_API USFZHierarchialOpcodeContainer : public UObject {
	GENERATED_BODY()

	UPROPERTY()
	USFZHierarchialOpcodeContainer* ParentPtr = nullptr;

	UPROPERTY()
	TArray< USFZHierarchialOpcodeContainer*> ChildPtrs;

	// these are the opcodes contained in this hierarchical structure
	UPROPERTY()
	TMap<FString, FString> OpcodesStringMap;
};
/**
 *
 */
UCLASS(Config = Editor, Category = "BK Music|SFZ|Factory")
class BK_EDITORUTILITIES_API USFZAssetFactory : public UFactory
{
	GENERATED_BODY()

	USFZAssetFactory();

	UFKSFZAsset* newSFZAsset;
	FString SFZFileName;

	// UFactory interface
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	// End of UFactory interface
public:

	bool createFusionPatch = true;
	EKeyzoneSelectMode KeyZoneSelectMode;

	FString UnrealSamplesSavePath;
	FString PathString;
	FString DefaultPath;
	int Octave_Offset;
	//needs to be made public
	static int32 ParseOpCodeValueToInt(const FString& Value);
	int NumFilesFailedToImport = 0;
	UPROPERTY()
	TMap<FName, USoundWave*> ImportedWavsMap;
	FSFZImportSettings SFZsettings;

	void ShowImportDialog(int32 ImportedAssetIndex)
	{
		//show the dialog widgets on a window
		TSharedPtr<SWindow> ParentWindow;
		SAssignNew(ParentWindow, SWindow)
			.Title(FText::FromString(TEXT("SFZ Import Options")))
			.ClientSize(FVector2D(490, 350));

		//the custom pop up dialog that asks for conforming midi file length
		TSharedRef<SImportSFZSettingsDialog> ImportSFZOptionsDialog = SNew(SImportSFZSettingsDialog);
		ParentWindow->SetContent(ImportSFZOptionsDialog);

		ImportSFZOptionsDialog->SFZFileInfoText->SetText(FText::FromString(FString::Printf(
			TEXT("%s \n num regions %d"),
			*SFZFileName,
			newSFZAsset->Regions.Num()
		)));

		//Set callback function for Ok button
		ImportSFZOptionsDialog->OkButton->SetOnClicked(FOnClicked::CreateLambda([this, ParentWindow, ImportSFZOptionsDialog]()-> FReply {
			ParentWindow->RequestDestroyWindow();
			this->SFZsettings = ImportSFZOptionsDialog->settings;
			return FReply::Handled();
			}));

		//show the dialog
		FSlateApplication::Get().AddModalWindow(ParentWindow.ToSharedRef(), nullptr);
	}

	//custom widgets
};
