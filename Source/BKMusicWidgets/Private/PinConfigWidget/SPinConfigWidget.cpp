// Fill out your copyright notice in the Description page of Project Settings.


#include "PinConfigWidget/SPinConfigWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Pins/M2Pins.h"
#include "Vertexes/M2SoundVertex.h"
#include "Widgets/Input/SNumericEntryBox.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPinConfigWidget::Construct(const FArguments& InArgs, const UM2Pins* InPin)
{

	Pin = InPin;

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
			];

		//if float, show min max
		if (AsLiteralPin->DataType == "Float")
		{
			MainCotentArea->AddSlot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Min:"))))
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<float>)
								.Value_Lambda([this]() {return MinValue; })
								.OnValueChanged_Lambda([this](float InValue) {MinValue = InValue;
								UpdateMinMax(MinValue, MaxValue); })
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Max:"))))
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<float>)
								.Value_Lambda([this]() {return MaxValue; })
								.OnValueChanged_Lambda([this](float InValue) {MaxValue = InValue;
							UpdateMinMax(MinValue, MaxValue); })
						]

				];
		}

		if (AsLiteralPin->DataType == "Int32")
			{
			MainCotentArea->AddSlot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Min:"))))
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<int32>)
								.Value(0)
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Max:"))))
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<int32>)
								.Value(1)
						]

				];
		}


	}

}

void SPinConfigWidget::UpdateMinMax(float InMin, float InMax)
{
	MinValue = InMin;
	MaxValue = InMax;

	FName SearchName = FName(Pin->ParentVertex->GetName());
	const auto& Settings = UUNDAWSettings::Get();
	Settings->Cache.Contains(SearchName);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

