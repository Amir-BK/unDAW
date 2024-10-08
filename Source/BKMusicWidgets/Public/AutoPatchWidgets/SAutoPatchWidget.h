// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Pins/M2Pins.h"
#include "MetasoundFrontendController.h"
#include "MetasoundFrontendDataTypeRegistry.h"
#include "HarmonixMetasound/DataTypes/MusicTimestamp.h"

#include "Vertexes/M2SoundVertex.h"







/**
 * the literal controller widget is a wrapper for the various types of audio widgets that can be used to control a literal,
 * using the bindings on the m2soundvertex and its pins this should point back to a MetasoundBuilderNodeInpput handle 
 */

class BKMUSICWIDGETS_API SM2LiteralControllerWidget : public SCompoundWidget
{
public:
		SLATE_BEGIN_ARGS(SM2LiteralControllerWidget)
		{}
		SLATE_END_ARGS()


		

		void Construct(const FArguments& InArgs, const UM2MetasoundLiteralPin& InLiteralPin);

		TSharedRef<SWidget> CreateValueWidget(const UM2MetasoundLiteralPin& InLiteralPin);

		TSharedRef<SWidget> MakeWidgetForEnumValue(TSharedPtr<FString> InOption);

		FText GetEnumValue() const;

		TSharedPtr<SHorizontalBox> MainHorizontalBox;

		FLinearColor PinColor = FLinearColor::White;

		const UM2MetasoundLiteralPin* LiteralPin;

		//possible frontend literal values... probably wasteful, maybe a good shout for templating

		TArray<UObject*> Objects;

		TSharedPtr<SWidget> ValueWidget;

		TArray<TSharedPtr<FString>> EnumOptions;

		TMap<TSharedPtr<FString>, int32> EnumOptionToValue;

		TArray<TSharedPtr<FString>> UObjectOptions;

		TSharedPtr<const Metasound::Frontend::IEnumDataTypeInterface> EnumInterface;

		FLinearColor Color = FLinearColor::White;

		int32 EnumValue = 0;


		float LiteralFloatValue = 0.0f;

		int32 LiteralIntValue = 0;

		FString LiteralStringValue = TEXT("");

		bool bLiteralBoolValue = false;

		UObject* LiteralObjectValue = nullptr;

		void UpdateValueForLiteralPin();

		void OnSelectObject(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

		void OnSelectEnum(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

		void OnLiteralValueChanged(float NewValue);

		void OnLiteralValueChanged(int32 NewValue);

		void OnLiteralValueChanged(const FString& NewValue);

		void OnLiteralValueChanged(const FText& NewText, ETextCommit::Type CommitInfo);

		void OnLiteralValueChanged(ECheckBoxState NewValue);

		void OnTimestampChanged(int32 bar, float beat);

		FReply ExecuteTriggerParameter();

		bool IsControlEnabled() const;

		void ExtractTimeStampFromLiteral();

		


};

/**
 *
 */
class BKMUSICWIDGETS_API SAutoPatchWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAutoPatchWidget)
		{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const UM2SoundVertex* InVertex);

	void OnVertexUpdated();

	UM2SoundPatch* GetSelectedPatch() const;



	private:
		TSharedPtr<SComboBox<TSharedPtr<FString>>> PatchComboBox;
		TArray<TSharedPtr<FString>> PatchOptions;
		TSharedPtr<SVerticalBox> ContentBox;
		const UM2SoundVertex* Vertex;
		UM2SoundPatch* SelectedPatch;

};
