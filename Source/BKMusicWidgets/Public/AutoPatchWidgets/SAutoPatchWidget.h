// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SComboBox.h"
#include "Pins/M2Pins.h"
#include "MetasoundFrontendController.h"
#include "MetasoundFrontendDataTypeRegistry.h"
#include "HarmonixMetasound/DataTypes/MusicTimestamp.h"
#include "unDAWSettings.h"
#include "UndawWidgetsSettings.h"
#include "AudioMaterialSlate/SAudioMaterialButton.h"
#include "AudioMaterialSlate/SAudioMaterialLabeledKnob.h"
#include "AudioMaterialSlate/SAudioMaterialLabeledSlider.h"


#include "Vertexes/M2SoundVertex.h"







/**
 * the literal controller widget is a wrapper for the various types of audio widgets that can be used to control a literal,
 * using the bindings on the m2soundvertex and its pins this should point back to a MetasoundBuilderNodeInpput handle 
 */

class BKMUSICWIDGETS_API SM2LiteralControllerWidget : public SCompoundWidget
{
public:
		SLATE_BEGIN_ARGS(SM2LiteralControllerWidget)
		{}
		SLATE_END_ARGS()


		

		void Construct(const FArguments& InArgs, const UM2MetasoundLiteralPin& InLiteralPin);

		TSharedRef<SWidget> CreateValueWidget(const UM2MetasoundLiteralPin& InLiteralPin);

		TSharedRef<SWidget> MakeWidgetForEnumValue(TSharedPtr<FString> InOption);

		FText GetEnumValue() const;

		TSharedPtr<SHorizontalBox> MainHorizontalBox;

		FLinearColor PinColor = FLinearColor::White;

		const UM2MetasoundLiteralPin* LiteralPin;

		//possible frontend literal values... probably wasteful, maybe a good shout for templating

		TArray<UObject*> Objects;

		TSharedPtr<SWidget> ValueWidget;

		TArray<TSharedPtr<FString>> EnumOptions;

		TMap<TSharedPtr<FString>, int32> EnumOptionToValue;

		TArray<TSharedPtr<FString>> UObjectOptions;

		TSharedPtr<const Metasound::Frontend::IEnumDataTypeInterface> EnumInterface;

		FLinearColor Color = FLinearColor::White;

		int32 EnumValue = 0;


		float LiteralFloatValue = 0.0f;

		int32 LiteralIntValue = 0;

		FString LiteralStringValue = TEXT("");

		bool bLiteralBoolValue = false;

		UObject* LiteralObjectValue = nullptr;

		void UpdateValueForLiteralPin();

		void OnSelectObject(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

		void OnSelectEnum(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

		void OnLiteralValueChanged(float NewValue);

		void OnLiteralValueChanged(int32 NewValue);

		void OnLiteralValueChanged(const FString& NewValue);

		void OnLiteralValueChanged(const FText& NewText, ETextCommit::Type CommitInfo);

		void OnLiteralValueChanged(ECheckBoxState NewValue);

		void OnTimestampChanged(int32 bar, float beat);

		FReply ExecuteTriggerParameter();

		bool IsControlEnabled() const;

		void ExtractTimeStampFromLiteral();

		


};

DECLARE_DELEGATE_TwoParams(FOnInnerWidgetDragged, const FVector2D, const SWidget*)


class BKMUSICWIDGETS_API SMaterialControllerFloatWidget : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SMaterialControllerFloatWidget)
		{}
		SLATE_EVENT(FOnInnerWidgetDragged, OnInnerWidgetDragged)
	SLATE_END_ARGS()

	bool bIsInit = false;

	void Construct(const FArguments& InArgs, const UM2MetasoundLiteralPin* InMaterialPin, const FM2SoundFloatPinConfig* InConfig)
	{
		LiteralPin = InMaterialPin;
		Config = InConfig;
		UObject* SliderStyle = nullptr;
		UObject* KnobStyle = nullptr;
		const FAudioMaterialSliderStyle* Style = nullptr;
		const FAudioMaterialKnobStyle* KnobStyleAsset = nullptr;

		OnInnerWidgetDragged = InArgs._OnInnerWidgetDragged;



		switch (InConfig->WidgetType)
		{
		case EFloatPinWidgetType::Slider:

			SliderStyle = InConfig->SliderStyleOverride.TryLoad();
	

			if (SliderStyle)
			{
				Style = CastChecked<USlateWidgetStyleAsset>(SliderStyle)->GetStyle<FAudioMaterialSliderStyle>();
			}
			else
			{
				Style = &FAudioWidgetsStyle::Get().GetWidgetStyle<FAudioMaterialSliderStyle>("AudioMaterialSlider.Style");
			}


			ChildSlot
				[
					SNew(SBorder)
						[
							SAssignNew(InputWidget, SAudioMaterialLabeledSlider)
								.SliderValue(this, &SMaterialControllerFloatWidget::GetValue)
								.OnValueChanged(this, &SMaterialControllerFloatWidget::OnValueChanged)
								.Style(Style)
								.AudioUnitsValueType(Config->UnitType)
								.Orientation(Config->SliderOrientation)
						]
			
				];

		break;

		case EFloatPinWidgetType::Knob:

			KnobStyle = InConfig->KnobStyleOverride.TryLoad();
	

			if (KnobStyle)
			{
				KnobStyleAsset = CastChecked<USlateWidgetStyleAsset>(KnobStyle)->GetStyle<FAudioMaterialKnobStyle>();
			}
			else
			{
				KnobStyleAsset = &FAudioWidgetsStyle::Get().GetWidgetStyle<FAudioMaterialKnobStyle>("AudioMaterialKnob.Style");
			}


			ChildSlot
				[
					SNew(SBorder)
						[
						SAssignNew(InputWidget, SAudioMaterialLabeledKnob)
							.Value(this, &SMaterialControllerFloatWidget::GetValue)
							.OnValueChanged(this, &SMaterialControllerFloatWidget::OnValueChanged)
							.AudioUnitsValueType(Config->UnitType)
							.Style(KnobStyleAsset)
						]
				];

			break;

		default:
		
		break;
		}

		bIsInit = true;

	}

	float GetValue() const {
		float Value;
		if (LiteralPin->LiteralValue.TryGet(Value))
		{
			UE_LOG(LogTemp, Warning, TEXT("Get Value: %f, Pin Name: %s"), Value, *LiteralPin->Name.ToString());
			return Value;
		}
		UE_LOG(LogTemp, Warning, TEXT("Get Value: 0.0f, Pin Name: %s"), *LiteralPin->Name.ToString());
		return 0.0f;
		 }

	void SetValue(float NewValue) 
	{
		EMetaSoundBuilderResult BuildResult;
		auto NonConstLiteralPin = const_cast<UM2MetasoundLiteralPin*>(LiteralPin);
		NonConstLiteralPin->LiteralValue.Set(InputWidget->GetOutputValue(NewValue));
		NonConstLiteralPin->ParentVertex->GetSequencerData()->BuilderContext->SetNodeInputDefault(LiteralPin->GetHandle<FMetaSoundBuilderNodeInputHandle>(), NonConstLiteralPin->LiteralValue, BuildResult);
	};

	void OnValueChanged(float NewValue) {
		UE_LOG(LogTemp, Warning, TEXT("Value Changed: %f, Pin Name: %s"), InputWidget->GetOutputValue(NewValue), *LiteralPin->Name.ToString());

		if (!bIsInit) return;

		EMetaSoundBuilderResult BuildResult;
		auto NonConstLiteralPin = const_cast<UM2MetasoundLiteralPin*>(LiteralPin);
		NonConstLiteralPin->LiteralValue.Set(InputWidget->GetOutputValue(NewValue));
		NonConstLiteralPin->ParentVertex->GetSequencerData()->BuilderContext->SetNodeInputDefault(LiteralPin->GetHandle<FMetaSoundBuilderNodeInputHandle>(), NonConstLiteralPin->LiteralValue, BuildResult);
	};

	FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	{
		//OnInnerWidgetDragged.ExecuteIfBound(MouseEvent.GetScreenSpacePosition(), this);
		UE_LOG(LogTemp, Warning, TEXT("Drag Detected"));
		return FReply::Handled();
	}

	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{	
		const bool bIsLeftMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton;
		//is left mouse button down?
		if (bIsLeftMouseButtonEffecting)
		{
			OnInnerWidgetDragged.ExecuteIfBound(MouseEvent.GetScreenSpacePosition(), this);
			return FReply::Handled().CaptureMouse(SharedThis(this));
		}

		return FReply::Unhandled();
	}

	const FM2SoundFloatPinConfig* Config = nullptr;

	const UM2MetasoundLiteralPin* LiteralPin;
//
	FOnInnerWidgetDragged OnInnerWidgetDragged;

	TSharedPtr<SAudioInputWidget> InputWidget;

	//void OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	//TSharedPtr<SAudioRadialSlider> Slider;

};


/**
 *
 */
class BKMUSICWIDGETS_API SAutoPatchWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAutoPatchWidget)
		{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const UM2SoundVertex* InVertex);

	//void OnVertexUpdated();

	UM2SoundPatch* GetSelectedPatch() const;

	FM2SoundFloatPinConfig* Config = nullptr;

	FName PatchName = NAME_None;

	private:
		TSharedPtr<SComboBox<TSharedPtr<FString>>> PatchComboBox;
		TArray<TSharedPtr<FString>> PatchOptions;
		TSharedPtr<SConstraintCanvas> ContentBox;
		const UM2SoundVertex* Vertex;
		UM2SoundPatch* SelectedPatch;

};
