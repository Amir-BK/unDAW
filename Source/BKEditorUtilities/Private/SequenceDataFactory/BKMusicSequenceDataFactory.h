// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "UObject/Object.h"
#include "BK_MusicSceneManagerInterface.h"
#include "AssetTypeActions_Base.h"
#include "../SequenceAssetEditor/UnDawSequenceEditorToolkit.h"
#include "SequencerData.h"
#include "BKMusicSequenceDataFactory.generated.h"


class FDAWSequenceAssetActions : public FAssetTypeActions_Base
{
public:
	UClass* GetSupportedClass() const override
	{
		return UDAWSequencerData::StaticClass();
	}
	FText GetName() const override
	{
		return INVTEXT("unDAW Session Data");
	}
	FColor GetTypeColor() const override 
	{
		return FColor::Purple;
	}
	uint32 GetCategories() override
	{
		return EAssetTypeCategories::Media;
	}

	void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor) override
	{
		MakeShared<FUnDAWSequenceEditorToolkit>()->InitEditor(InObjects);
	}
};


/**
 * 
 */
UCLASS()
class BK_EDITORUTILITIES_API UBKMusicSequenceDataFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	//~ UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;



	virtual bool ShouldShowInNewMenu() const override;

	UBKMusicSequenceDataFactory();

	

};

