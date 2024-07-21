// Fill out your copyright notice in the Description page of Project Settings.


#include "PinConfigWidget/SPinConfigWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPinConfigWidget::Construct(const FArguments& InArgs, const UM2Pins* InPin)
{

ChildSlot
[
	SNew(STextBlock)
		.Text(FText::FromString(InPin->Name.ToString()))
		.ColorAndOpacity(FLinearColor::Yellow)
];


}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

