// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoPatchWidgets/SVariMixerWidget.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SVariMixerWidget::Construct(const FArguments& InArgs)
{
	
	ChildSlot
	[
		SNew(STextBlock)
			.Text(FText::FromString("VariMixerWidget"))
	];
	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
