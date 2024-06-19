// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Components/Slider.h"
#include "BK_MusicSceneManagerInterface.h"
#include "GlyphButton.h"
#include "Components/TextBlock.h"
#include "M2SoundGraphData.h"
#include "UnDAWWidgetBase.h"
#include "SceneManagerTransport.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeekValueChangedEvent, float, Value);

//macro to make declaring the transport actions easier
#define TRANSPORTACTION(Button)		if (Button) { Button->TransportButtonClicked.AddUniqueDynamic(this, &USceneManagerTransportWidget::ReceiveButtonClick);}
/**
 * Base class for the scene manager transport, you have to inherit this in UMG and use the button binds to make it work. See the BP example.
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class BKMUSICWIDGETS_API USceneManagerTransportWidget : public UUnDAWWidgetBase
{
	GENERATED_BODY()

public: 
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "unDAW|Transport")
	FOnTransportCommand TransportCalled;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "unDAW|Transport")
	FOnSeekValueChangedEvent TransportSeekCommand;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Transport")
	TEnumAsByte<EBKPlayState> TransportPlayState;

	UFUNCTION()
	void ReceiveButtonClick(EBKTransportCommands newCommand)
	{
		UE_LOG(LogTemp, Log, TEXT("%s send command %s"), *this->GetName(), *UEnum::GetValueAsString(newCommand))
			SceneManager->SendTransportCommand(newCommand);
		//TransportCalled.Broadcast(newCommand);
	}

	UFUNCTION()
	void OnTimestampUpdated(FMusicTimestamp NewTimestamp)
	{
		auto tick = DawSequencerData->HarmonixMidiFile->GetSongMaps()->CalculateMidiTick(NewTimestamp, EMidiClockSubdivisionQuantization::None);
		auto sec = DawSequencerData->HarmonixMidiFile->GetSongMaps()->TickToMs(tick) * .001f ;

		CurrentSeek = sec;
		if (PlayPosition) PlayPosition->SetValue(sec);
		if (CurrentPositionText) CurrentPositionText->SetText(FText::AsNumber(sec));

	}


protected:



	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class USlider* PlayPosition;

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

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* DurationText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* CurrentPositionText;

	UPROPERTY()
	float CurrentSeek = 0;

	//UPROPERTY(BlueprintReadOnly)
	//TScriptInterface<IBK_MusicSceneManagerInterface> SceneManager = nullptr;

public:

	
	void Init() override;

	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport")
	void SetTransportDuration(float newDuration)
	{
		if (PlayPosition) PlayPosition->SetMaxValue(newDuration);
		if (DurationText) DurationText->SetText(FText::AsNumber(newDuration));
	}

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "unDAW|Transport")
	void SetTransportSeek(float NewSeek)
	{
		if (NewSeek != CurrentSeek)
		{
			if (SceneManager) SceneManager->SendSeekCommand(NewSeek);
		}

	}

	UFUNCTION(BlueprintCallable, Category = "unDAW|Transport")
	void SetTransportPlayState(EBKPlayState newPlayState)
	{
		TransportPlayState = newPlayState;
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

		if (PlayPosition)
			{
			PlayPosition->OnValueChanged.AddUniqueDynamic(this, &USceneManagerTransportWidget::SetTransportSeek);
			}
	}
};

#undef TRANSPORTACTION