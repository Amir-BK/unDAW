// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorSlateWidgets/SM2SoundGraphPin.h"
#include "SlateOptMacros.h"
#include "SPinTypeSelector.h"
#include "AutoPatchWidgets/SAutoPatchWidget.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SM2SoundGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	

	LiteralPin = Cast<UM2MetasoundLiteralPin>(InGraphPinObj->PinType.PinSubCategoryObject.Get());


	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
	//bUsePinColorForText = true; 

	// Create the pin icon widget
	TSharedRef<SWidget> PinWidgetRef = SPinTypeSelector::ConstructPinTypeImage(
		MakeAttributeSP(this, &SM2SoundGraphPin::GetPinIcon),
		MakeAttributeSP(this, &SM2SoundGraphPin::GetPinColor),
		MakeAttributeSP(this, &SM2SoundGraphPin::GetSecondaryPinIcon),
		MakeAttributeSP(this, &SM2SoundGraphPin::GetSecondaryPinColor));
	PinImage = PinWidgetRef;
	
}
TSharedRef<SWidget> SM2SoundGraphPin::GetDefaultValueWidget()
{
	if(LiteralPin != nullptr)
	{
		auto TempValWidget =  SNew(SM2LiteralControllerWidget, *LiteralPin);
		PinColor = TempValWidget->PinColor;

		return TempValWidget;
	}
	else {
		return SNullWidget::NullWidget;
	}
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION