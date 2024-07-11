// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Selection.h"
#include "Editor/Blutility/Classes/EditorUtilityWidget.h"
#include "ListenerComponent/DAWListenerComponent.h"
#include "MusicScenePlayerActor.h"
#include "Components/DetailsView.h"
#include "Widgets/SBoxPanel.h"
#include "BKListnerComponentConfigWidget.generated.h"

/**
 * 
 */
UCLASS()
class BK_EDITORUTILITIES_API UBKListnerComponentConfigWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()


	virtual void OnObjectSlected(UObject* SelectedObject);

	virtual void OnSelectNone();

	void InitFromData();



	UPROPERTY()
	TObjectPtr<AMusicScenePlayerActor> DefaultSceneManager;

	UPROPERTY()
	UDAWListenerComponent* ControlledComponent;


	TSharedPtr<SVerticalBox> MainViewArea;



public:

	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly)
	FString SelectedObjectName = "";

protected:


	UFUNCTION(BlueprintImplementableEvent, CallInEditor)
	void OnListenerComponentSelected(UDAWListenerComponent* SelectedComponent);


	//UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	//class UDetailsView* ComponentDetailsView;


	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	//virtual void NativeConstruct() override; 

	//~ End UWidget Interface
	
};
