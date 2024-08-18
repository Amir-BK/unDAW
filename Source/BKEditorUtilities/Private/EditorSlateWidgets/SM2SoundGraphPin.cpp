// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorSlateWidgets/SM2SoundGraphPin.h"
#include "SlateOptMacros.h"
#include "AutoPatchWidgets/SAutoPatchWidget.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SM2SoundGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);

	auto M2SoundLiteralSubCategoryObject = Cast<UM2MetasoundLiteralPin>(InGraphPinObj->PinType.PinSubCategoryObject.Get());

	if(M2SoundLiteralSubCategoryObject == nullptr)
	{
		return;
	}

	LabelAndValue.Get()->ClearChildren();
	//LabelAndValue.Get()->
	LabelAndValue.Get()->AddSlot().AttachWidget(SNew(SM2LiteralControllerWidget, *M2SoundLiteralSubCategoryObject));
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION