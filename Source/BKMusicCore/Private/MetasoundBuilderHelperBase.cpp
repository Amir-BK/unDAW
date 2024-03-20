// Fill out your copyright notice in the Description page of Project Settings.


#include "MetasoundBuilderHelperBase.h"

void UMetasoundBuilderHelperBase::InitBuilderHelepr()
{
	UE_LOG(LogTemp,Log, TEXT("Test"))

	MSBuilderSystem = GEngine->GetEngineSubsystem<UMetaSoundBuilderSubsystem>();
}
