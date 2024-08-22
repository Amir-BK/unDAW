// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"
#include "Widgets/SCompoundWidget.h"
#include <Pins/M2Pins.h>

/**
 *
 */
class BK_EDITORUTILITIES_API SM2SoundGraphPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SM2SoundGraphPin)
		{}
	SLATE_END_ARGS()

	UM2MetasoundLiteralPin* LiteralPin = nullptr;

	FLinearColor PinColor = FLinearColor::White;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

	TSharedRef<SWidget> GetDefaultValueWidget() override;

	FSlateColor GetPinColor() const override
	{
		return PinColor;
	}

};
