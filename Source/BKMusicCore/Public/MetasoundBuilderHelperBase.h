// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MetasoundBuilderSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/AudioComponent.h"
#include "Metasound.h"
#include "MetasoundSource.h"
#include "SequencerData.h"
#include "MetasoundBuilderHelperBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class BKMUSICCORE_API UMetasoundBuilderHelperBase : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, Category = "unDAW|MetaSound Builder Helper")
	UAudioComponent* AuditionComponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	FMetaSoundBuilderNodeOutputHandle OnPlayOutputNode;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper", meta = (ExposeOnSpawn = true))
	UDAWSequencerData* SessionData;
	
	UFUNCTION(BlueprintImplementableEvent)
	void PerformBpInitialization();

	UFUNCTION(BlueprintCallable, Category = "unDAW|MetaSound Builder Helper", CallInEditor)
	void InitBuilderHelper(FString BuilderName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "unDAW|MetaSound Builder Helper")
	EMetaSoundOutputAudioFormat OutputFormat;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	UMetaSoundBuilderSubsystem* MSBuilderSystem;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	UMetaSoundSourceBuilder* CurrentBuilder;

	//can be used to get all assets of certain class
	template<typename T>
	static void GetObjectsOfClass(TArray<T*>& OutArray);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "unDAW|MetaSound Builder Helper")
	static TArray<UMetaSoundSource*> GetAllMetasoundSourcesWithInstrumentInterface();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "unDAW|MetaSound Builder Helper")
	static TArray<UMetaSoundPatch*> GetAllMetasoundPatchesWithInstrumentInterface();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "unDAW|MetaSound Builder Helper")
	static TArray<UMetaSoundPatch*> GetAllMetasoundPatchesWithInsertInterface();


	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper");
	TScriptInterface<IMetaSoundDocumentInterface> GeneratedMetaSound;

	UFUNCTION()
	void AuditionAC(UAudioComponent* AudioComponent);


};

template<typename T>
inline void UMetasoundBuilderHelperBase::GetObjectsOfClass(TArray<T*>& OutArray)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssetsByClass(T::StaticClass()->GetClassPathName(), AssetData);
	for (int i = 0; i < AssetData.Num(); i++) {
		T* Object = Cast<T>(AssetData[i].GetAsset());
		OutArray.Add(Object);
	}
}
