// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SConstraintCanvas.h"

/**
 * 
 */
class BKMUSICWIDGETS_API SunDAWAudioTrack : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SunDAWAudioTrack)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	TSharedPtr<SConstraintCanvas> RootConstraintCanvas;
};
