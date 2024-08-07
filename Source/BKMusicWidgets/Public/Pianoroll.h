// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnDAWWidgetBase.h"
#include "MusicScenePlayerActor.h"
#include "SPianoRollGraph.h"
#include "Pianoroll.generated.h"


UCLASS(BlueprintType, Blueprintable)
class BKMUSICWIDGETS_API UPianorollPalette : public UUnDAWWidgetBase
{
	GENERATED_BODY()


	public:

		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pianoroll")
		TObjectPtr<UPianoroll> Pianoroll;

};

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
	float PianoTabMargin = 50.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pianoroll")
	float DesiredCursorPositionOnScreen = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pianoroll")
	TSubclassOf<UPianorollPalette> PaletteClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pianoroll")
	TObjectPtr<UPianorollPalette> Palette;

	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	TSharedPtr<SPianoRollGraph> PianoRollGraph;

	//UFUNCTION(BlueprintCallable, Category = "Pianoroll")
	//virtual UPianorollPalette* GetPalette() {};
};
