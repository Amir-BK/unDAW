// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MusicScenePlayerActor.h"
#include "UnDAWWidgetBase.generated.h"

/**
 *
 */
UCLASS()
class BKMUSICWIDGETS_API UUnDAWWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void SetSceneManager(AMusicScenePlayerActor* InSceneManager)
	{
		SceneManager = InSceneManager;

		if (!SceneManager) return;
		DawSequencerData = SceneManager->GetDAWSequencerData();

		Init();
	}

	UFUNCTION()
	virtual void Init() {};

protected:
	//Scene manager reference
	UPROPERTY()
	TObjectPtr < AMusicScenePlayerActor> SceneManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW", Meta = (DisplayName = "DAW Sequencer Data"))
	TObjectPtr<UDAWSequencerData> DawSequencerData;
};
