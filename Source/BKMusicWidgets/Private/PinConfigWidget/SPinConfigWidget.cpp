// Fill out your copyright notice in the Description page of Project Settings.


#include "PinConfigWidget/SPinConfigWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPinConfigWidget::Construct(const FArguments& InArgs, const UM2Pins* InPin)
{

	ChildSlot
		[
			SAssignNew(MainCotentArea, SVerticalBox)
		];

	//if literal
	if (auto* AsLiteralPin = Cast<UM2MetasoundLiteralPin>(InPin))
	{
		MainCotentArea->AddSlot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("Literal Pin: %s"), *AsLiteralPin->Name.ToString())))
			]
			[
				SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("Data Type: %s"), *AsLiteralPin->DataType.ToString())))
			];


	}

}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

