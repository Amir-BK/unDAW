// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "HarmonixDsp/FusionSampler/FusionPatch.h"
#include "XmlFile.h"
#include <XmlNode.h>

#include "BKDPresetToFusionImporter.generated.h"

/**
 *
 */
UCLASS()
class BK_EDITORUTILITIES_API UBKDPresetToFusionImporter : public UFactory
{
	GENERATED_BODY()

public:

	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;

	TArray <FKeyzoneSettings> KeyZoneArray;
	FFusionPatchSettings newSettings;

	UBKDPresetToFusionImporter();

	const FXmlNode* currentGroup;
	static void traverseXML(const FXmlNode* nodeIn, TArray <FKeyzoneSettings>& workingArray, FFusionPatchSettings& workingSettings);
	static void configureFusionPatchFromDSPresetData(UFusionPatch* inPatch, TArray <FKeyzoneSettings>& workingArray, FFusionPatchSettings& workingSettings);
};
