// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MetasoundBuilderSubsystem.h"
#include "MetasoundBuilderHelperBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class BKMUSICCORE_API UMetasoundBuilderHelperBase : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void PerformBpInitialization();

	UFUNCTION(BlueprintCallable)
	void InitBuilderHelepr();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	EMetaSoundOutputAudioFormat OutputFormat;

	UPROPERTY(BlueprintReadOnly)
	UMetaSoundBuilderSubsystem* MSBuilderSystem;


};
