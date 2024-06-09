// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Metasound.h"
#include "AssetRegistry/AssetRegistryModule.h"

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

	/**
	* Get all the vertexes in the sequencer data
	 * @param SequencerData The sequencer data to get the vertexes from
	 * @return An array of all the vertexes in the sequencer data
	 */

	static TArray<UM2SoundVertex*> GetAllVertexesInSequencerData(UDAWSequencerData* SequencerData);
	
	//can be used to get all assets of certain class
	template<typename T>
	static void GetObjectsOfClass(TArray<T*>& OutArray);

	static UMetaSoundPatch* GetPatchByName(FString Name);



	/**
	* Get All Metasound Patches that implement the unDAW instrument parameter interface
	* @return An array of all the patches that implement the instrument interface
	* */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "unDAW|MetaSound Builder Helper")
	static TArray<UMetaSoundPatch*> GetAllPatchesImplementingInstrumetInterface();

	/**
	* Get All Metasound Patches that implement the unDAW insert parameter interface
	* @return An array of all the patches that implement the insert interface
	* */

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "unDAW|MetaSound Builder Helper")
	static TArray<UMetaSoundPatch*> GetAllMetasoundPatchesWithInsertInterface();

	static bool DoesPatchImplementInterface(UMetaSoundPatch* Patch, UClass* InterfaceClass);
	
};

template<typename T>
inline void UM2SoundGraphStatics::GetObjectsOfClass(TArray<T*>& OutArray)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssetsByClass(T::StaticClass()->GetClassPathName(), AssetData);
	for (int i = 0; i < AssetData.Num(); i++) {
		T* Object = Cast<T>(AssetData[i].GetAsset());
		OutArray.Add(Object);
	}
}
