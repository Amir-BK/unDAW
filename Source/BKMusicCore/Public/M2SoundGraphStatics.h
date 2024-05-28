// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "M2SoundGraphStatics.generated.h"


class UDAWSequencerData;
class UM2SoundVertex;
class UM2SoundTrackInput;

/**
 * Static and Blueprint functions for M2SoundGraph, hopefully in the future these will also be used for in game representation of the graph (unPatchWork)
 */
UCLASS()
class BKMUSICCORE_API UM2SoundGraphStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:


	/**
	 * Initialize a connected channel in the sequencer data, this is used when 
	 * @param SequencerData The sequencer data to add the vertex to
	 * @param Vertex The vertex to start from
	 * @param the unique index of the track we're initializing
	 */
	static void CreateDefaultVertexesFromInputVertex(UDAWSequencerData* SequencerData, UM2SoundTrackInput* InputVertex, const int Index);
	
	
};
