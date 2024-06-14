// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HarmonixMidi/MidiFile.h"
#include "EditableMidiFile.generated.h"

/**
 * 
 */
UCLASS()
class BKMUSICCORE_API UEditableMidiFile : public UMidiFile
{
	GENERATED_BODY()
	

	public:
		void LoadFromHarmonixBaseFile(UMidiFile* BaseFile);

		void FinishRebuildingMidiFile();
};
