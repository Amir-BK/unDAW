// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "BK_MusicSceneManagerInterface.h"
#include "GlyphButton.h"
#include "SceneManagerTransport.generated.h"

//macro to make declaring the transport actions easier

#define TRANSPORTACTION(Button)		if (Button) { Button->TransportButtonClicked.AddUniqueDynamic(this, &USceneManagerTransportWidget::ReceiveButtonClick);}
/**
 * Base class for the scene manager transport, you have to inherit this in UMG and use the button binds to make it work. See the BP example.
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class BKMUSICWIDGETS_API USceneManagerTransportWidget : public UUserWidget
{
	GENERATED_BODY()

public: 

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "unDAW|Transport")
	FOnTransportCommand TransportCalled;


	

protected:

	UFUNCTION()
	void ReceiveButtonClick(EBKTransportCommands newCommand)
	{
		UE_LOG(LogTemp, Log, TEXT("We get in here and that's good enough"))
		TransportCalled.Broadcast(newCommand);
	}

	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();
		
		bIsVariable = true;

		TRANSPORTACTION(PauseButton)
		TRANSPORTACTION(InitButton)
		TRANSPORTACTION(StopButton)
		TRANSPORTACTION(KillButton)
		TRANSPORTACTION(PlayButton)
		
	}

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTransportGlyphButton* PlayButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTransportGlyphButton* PauseButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTransportGlyphButton* InitButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTransportGlyphButton* StopButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTransportGlyphButton* KillButton;


};

#undef TRANSPORTACTION