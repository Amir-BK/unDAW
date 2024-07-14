// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Pins/M2Pins.h"
#include "MetasoundFrontendController.h"
#include "MetasoundFrontendDataTypeRegistry.h"
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

		TSharedPtr<SHorizontalBox> MainHorizontalBox;

		FLinearColor PinColor = FLinearColor::White;

		TSharedPtr<SWidget> ValueWidget;

		TArray<TSharedPtr<FString>> EnumOptions;
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
