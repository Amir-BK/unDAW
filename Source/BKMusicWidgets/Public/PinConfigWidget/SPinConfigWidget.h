// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pins/M2Pins.h"
#include "Widgets/SBoxPanel.h"
#include "unDAWSettings.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class BKMUSICWIDGETS_API SPinConfigWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPinConfigWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const UM2Pins* InPin);

	float MinValue = 0.0f;

	float MaxValue = 1.0f;

	void UpdateMinMax(float InMin, float InMax);

	const UM2Pins* Pin;

	TSharedPtr<SVerticalBox> MainCotentArea;
};
