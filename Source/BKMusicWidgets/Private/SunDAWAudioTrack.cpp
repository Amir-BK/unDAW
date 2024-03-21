// Fill out your copyright notice in the Description page of Project Settings.


#include "SunDAWAudioTrack.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SunDAWAudioTrack::Construct(const FArguments& InArgs)
{

	ChildSlot
		[
			SAssignNew(RootConstraintCanvas, SConstraintCanvas)
		];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
