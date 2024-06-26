// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnDAWWidgetBase.h"
#include "MusicScenePlayerActor.h"
#include "SPianoRollGraph.h"
#include "Pianoroll.generated.h"

/**
 * Trying to use the spianoroll graph in game 
 */
UCLASS(BlueprintType, Blueprintable)
class BKMUSICWIDGETS_API UPianoroll : public UUnDAWWidgetBase
{
	GENERATED_BODY()
	
	UDAWSequencerData* SequencerData;

	public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pianoroll")
	float PianoTabMargin = 100.0f;

	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	TSharedPtr<SPianoRollGraph> PianoRollGraph;

};
