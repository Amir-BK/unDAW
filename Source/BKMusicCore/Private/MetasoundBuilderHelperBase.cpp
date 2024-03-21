// Fill out your copyright notice in the Description page of Project Settings.


#include "MetasoundBuilderHelperBase.h"
#include "Engine/Engine.h"
#include "IAudioParameterInterfaceRegistry.h"

#include "Interfaces/unDAWMetasoundInterfaces.h"


void UMetasoundBuilderHelperBase::InitBuilderHelper(FString BuilderName, EMetaSoundOutputAudioFormat SourceOutputFormat)
{
	UE_LOG(LogTemp,Log, TEXT("Test"))

	MSBuilderSystem = GEngine->GetEngineSubsystem<UMetaSoundBuilderSubsystem>();

	FMetaSoundBuilderNodeOutputHandle OnPlayOutputNode;
	FMetaSoundBuilderNodeInputHandle OnFinished;
	TArray<FMetaSoundBuilderNodeInputHandle> AudioOuts;
	EMetaSoundBuilderResult BuildResult;

	CurrentBuilder = MSBuilderSystem->CreateSourceBuilder(FName(BuilderName), OnPlayOutputNode, OnFinished, AudioOuts, BuildResult, OutputFormat, false);

}

TArray<UMetaSoundSource*> UMetasoundBuilderHelperBase::GetAllMetasoundSourcesWithInstrumentInterface()
{
	auto AllObjectsArray = TArray<UMetaSoundSource*>();
	auto OnlyImplementingArray = TArray<UMetaSoundSource*>();
	

	GetObjectsOfClass<UMetaSoundSource>(AllObjectsArray);
	for (const auto& source : AllObjectsArray)
	{
		if (source->ImplementsParameterInterface(unDAW::Metasounds::FunDAWInstrumentRendererInterface::GetInterface())) OnlyImplementingArray.Add(source);
	}
	
	return OnlyImplementingArray;
}

TArray<UMetaSoundPatch*> UMetasoundBuilderHelperBase::GetAllMetasoundPatchesWithInstrumentInterface()
{
	auto AllObjectsArray = TArray<UMetaSoundPatch*>();
	auto OnlyImplementingArray = TArray<UMetaSoundPatch*>();
	GetObjectsOfClass<UMetaSoundPatch>(AllObjectsArray);
	auto interface = unDAW::Metasounds::FunDAWInstrumentRendererInterface::GetInterface();

	const FMetasoundFrontendVersion Version{ interface->GetName(), { interface->GetVersion().Major, interface->GetVersion().Minor } };

	for (const auto& patch : AllObjectsArray)
	{
		if (patch->GetDocumentChecked().Interfaces.Contains(Version)) OnlyImplementingArray.Add(patch);
	}
	return OnlyImplementingArray;
}

TArray<UMetaSoundPatch*> UMetasoundBuilderHelperBase::GetAllMetasoundPatchesWithInsertInterface()
{
	auto AllObjectsArray = TArray<UMetaSoundPatch*>();
	auto OnlyImplementingArray = TArray<UMetaSoundPatch*>();
	GetObjectsOfClass<UMetaSoundPatch>(AllObjectsArray);
	auto interface = unDAW::Metasounds::FunDAWCustomInsertInterface::GetInterface();

	const FMetasoundFrontendVersion Version{ interface->GetName(), { interface->GetVersion().Major, interface->GetVersion().Minor } };

	for (const auto& patch : AllObjectsArray)
	{
		if (patch->GetDocumentChecked().Interfaces.Contains(Version)) OnlyImplementingArray.Add(patch);
	}
	return OnlyImplementingArray;
}


