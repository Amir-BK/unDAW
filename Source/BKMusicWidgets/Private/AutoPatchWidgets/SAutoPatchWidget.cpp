// Fill out your copyright notice in the Description page of Project Settings.

#include "AutoPatchWidgets/SAutoPatchWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSpacer.h"
#include "unDAWSettings.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SButton.h"
#include "SlateOptMacros.h"
#include "MetasoundBuilderSubsystem.h"
#include "M2SoundGraphStatics.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAutoPatchWidget::Construct(const FArguments& InArgs, const UM2SoundVertex* InVertex)
{
	Vertex = InVertex;

	ChildSlot
		[
			SAssignNew(ContentBox, SVerticalBox)
		];

	for (const auto& [Name, Pin] : Vertex->InputM2SoundPins)
	{
		if (Pin->IsA<UM2MetasoundLiteralPin>())
		{
			ContentBox->AddSlot()
				[
					SNew(SM2LiteralControllerWidget, *Cast<UM2MetasoundLiteralPin>(Pin))
				];
		}
		//else if (Pin->IsA<UM2MetasoundWaveTablePin>())
		//{
		//	ContentBox->AddSlot()
		//	[
		//		SNew(SM2WaveTableControllerWidget, *Cast<UM2MetasoundWaveTablePin>(Pin))
		//	];
		//}
		//else if (Pin->IsA<UM2MetasoundTriggerPin>())
		//{
		//	ContentBox->AddSlot()
		//	[
		//		SNew(SM2TriggerControllerWidget, *Cast<UM2MetasoundTriggerPin>(Pin))
		//	];
		//}
		//else if (Pin->IsA<UM2MetasoundDataReferencePin>())
		//{
		//	ContentBox->AddSlot()
		//	[
		//		SNew(SM2DataReferenceControllerWidget, *Cast<UM2MetasoundDataReferencePin>(Pin))
		//	];
		//}
		//else if (Pin->IsA<UM2MetasoundAudioBufferPin>())
		//{
		//	ContentBox->AddSlot()
		//	[
		//		SNew(SM2AudioBufferControllerWidget, *Cast<UM2MetasoundAudioBufferPin>(Pin))
		//	];
		//}
		//else if (Pin->IsA<UM2MetasoundTimePin>())
		//{
		//	ContentBox->AddSlot()
		//	[
		//		SNew(SM2TimeControllerWidget, *Cast<UM2MetasoundTimePin>(Pin))
		//	];
		//}
		else
		{
			ContentBox->AddSlot()
				[
					SNew(STextBlock)
						.Text(FText::FromString(Pin->GetName()))
				];
		}
	}
}
UM2SoundPatch* SAutoPatchWidget::GetSelectedPatch() const
{
	return nullptr;
}

void SM2LiteralControllerWidget::Construct(const FArguments& InArgs, const UM2MetasoundLiteralPin& InLiteralPin)
{
	using namespace Metasound::Frontend;
	FDataTypeRegistryInfo Info;

	LiteralPin = &InLiteralPin;

	Metasound::Frontend::IDataTypeRegistry::Get().GetDataTypeInfo(InLiteralPin.DataType, Info);
	//MetaSoundDataRegistry.;

	//UMetaSoundBuilderSubsystem* MetaSoundBuilderSubsystem = GEngine->GetEngineSubsystem<UMetaSoundBuilderSubsystem>();
	UMetaSoundSourceBuilder* MetaSoundSourceBuilder = InLiteralPin.ParentVertex->GetSequencerData()->BuilderContext;

	FLinearColor Color = FLinearColor::White;




	//get undaw settings and use colors from there
	auto Settings = UUNDAWSettings::Get();

	using namespace Metasound;

	ChildSlot
		[
			SAssignNew(MainHorizontalBox, SHorizontalBox)
				.IsEnabled(this, &SM2LiteralControllerWidget::IsControlEnabled)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
						.Text(FText::FromName(InLiteralPin.Name))
						.ColorAndOpacity_Lambda([this]() -> FSlateColor { return PinColor; })
						.ToolTipText(FText::FromString(FString::Printf(TEXT("%s %s"), *Info.DataTypeDisplayText.ToString(), LiteralPin->IsConstructorPin() ? TEXT(", Construcor Pin") : TEXT(""))))
				]
				//a spacer
				+ SHorizontalBox::Slot()
				[
					SNew(SSpacer)
						.Size(FVector2D(10, 0))
				]

		];
	

	switch (Info.PreferredLiteralType)
	{
	case ELiteralType::Boolean:
		if (Info.DataTypeName == FName("Trigger"))
		{
			PinColor = Settings->TriggerPinTypeColor;
			//simple SButton for trigger
			MainHorizontalBox->AddSlot()
				[
					SNew(SButton)
						.Text(FText::FromString(TEXT("Trigger")))
						.OnClicked(this, &SM2LiteralControllerWidget::ExecuteTriggerParameter)
						
				];



			break;
		}
		PinColor = Settings->BooleanPinTypeColor;
		MainHorizontalBox->AddSlot()
			[
				SNew(SCheckBox)
					.IsChecked_Lambda([this]() -> ECheckBoxState { return bLiteralBoolValue ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged(this, &SM2LiteralControllerWidget::OnLiteralValueChanged)
			//	.IsChecked_Lambda([this, &InLiteralPin]() -> ECheckBoxState { return InLiteralPin.GetBoolValue() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				//.OnCheckStateChanged_Lambda([this, &InLiteralPin](ECheckBoxState NewState) { InLiteralPin.SetBoolValue(NewState == ECheckBoxState::Checked); })
			];
		break;

	case ELiteralType::Float:
		PinColor = Settings->FloatPinTypeColor;
		MainHorizontalBox->AddSlot()
			[
				SNew(SNumericEntryBox<float>)
					.AllowSpin(true)
					.OnValueChanged(this, &SM2LiteralControllerWidget::OnLiteralValueChanged)
					.Value_Lambda([this]() -> TOptional<float> { return LiteralFloatValue; })
				//.Value_Lambda([this, &InLiteralPin]() -> TOptional<float> { return InLiteralPin.GetFloatValue(); })
				//.OnValueCommitted_Lambda([this, &InLiteralPin](float NewValue, ETextCommit::Type CommitType) { InLiteralPin.SetFloatValue(NewValue); })
			];

		break;
	case ELiteralType::Integer:
		if (Info.bIsEnum)
		{
			PinColor = Settings->IntPinTypeColor;
			EnumInterface = Metasound::Frontend::IDataTypeRegistry::Get().GetEnumInterfaceForDataType(InLiteralPin.DataType);
			auto DefaultEnumOption = EnumInterface->GetDefaultValue();
			//TArray<TSharedPtr<FString>> EnumOptions;
			for (int i = 0; i < EnumInterface->GetAllEntries().Num(); i++)
			{
				EnumOptions.Add(MakeShared<FString>(EnumInterface->ToName(i)->ToString()));
			}

			MainHorizontalBox->AddSlot()
				.AutoWidth()
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&EnumOptions)
					.InitiallySelectedItem(EnumOptions[0])
					.OnGenerateWidget_Lambda([this](TSharedPtr<FString> InOption) { return MakeWidgetForEnumValue(InOption); })
						[
						SNew(STextBlock)
							.Text_Lambda([this]() -> FText { return FText::FromName(EnumInterface->ToName(LiteralIntValue).GetValue()); })
						]	
						.OnSelectionChanged(this, &SM2LiteralControllerWidget::OnSelectEnum)

					//.OnSelectionChanged_Lambda([this, &InLiteralPin](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo) { InLiteralPin.SetIntValue(EnumInterface->FromName(*NewSelection)); })
				];
		}
		else
		{
			PinColor = Settings->IntPinTypeColor;
			MainHorizontalBox->AddSlot()
				[
					SNew(SNumericEntryBox<int32>)
						.AllowSpin(true)
						.Value_Lambda([this]() -> TOptional<int32> { return LiteralIntValue; })
						.OnValueChanged(this, &SM2LiteralControllerWidget::OnLiteralValueChanged)
					//.Value_Lambda([this, &InLiteralPin]() -> TOptional<int32> { return InLiteralPin.GetIntValue(); })
					//.OnValueCommitted_Lambda([this, &InLiteralPin](int32 NewValue, ETextCommit::Type CommitType) { InLiteralPin.SetIntValue(NewValue); })
				];
		}

		break;
	case ELiteralType::String:
		PinColor = Settings->StringPinTypeColor;
		break;
	case ELiteralType::UObjectProxy:
		Objects = UM2SoundGraphStatics::GetAllObjectsOfClass(Info.ProxyGeneratorClass);

		for (UObject* Object : Objects)
		{
			UObjectOptions.Add(MakeShared<FString>(Object->GetName()));
		}

		MainHorizontalBox->AddSlot()
			.AutoWidth()
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&UObjectOptions)
				.InitiallySelectedItem(UObjectOptions.IsEmpty() ? nullptr : UObjectOptions[0])
				.OnGenerateWidget_Lambda([this](TSharedPtr<FString> InOption) { return MakeWidgetForEnumValue(InOption); })
				[
					SNew(STextBlock)
						.Text_Lambda([this]() -> FText { return FText::FromString(LiteralObjectValue ? LiteralObjectValue->GetName() : TEXT("None")); })
				]
				//	.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo) { LiteralObjectValue = Objects[0]; })
					.OnSelectionChanged(this, &SM2LiteralControllerWidget::OnSelectObject)
			];


		PinColor = Settings->ObjectPinTypeColor;
		break;

	default:
		break;
	}

	//won't actually work, we need to construct an FTimeStamp... 
	if (Info.DataTypeName == FName("MusicTimeStamp"))
	{
		MainHorizontalBox->AddSlot()
			[
				SNew(SNumericEntryBox<float>)
				.AllowSpin(true)
				.OnValueChanged(this, &SM2LiteralControllerWidget::OnLiteralValueChanged)
				.Value_Lambda([this]() -> TOptional<float> { return LiteralFloatValue; })
			];

		MainHorizontalBox->AddSlot()
			[
				SNew(SNumericEntryBox<int32>)
				.AllowSpin(true)
				.OnValueChanged(this, &SM2LiteralControllerWidget::OnLiteralValueChanged)
				.Value_Lambda([this]() -> TOptional<int32> { return LiteralIntValue; })
			];
	}



}

TSharedRef<SWidget> SM2LiteralControllerWidget::CreateValueWidget(const UM2MetasoundLiteralPin& InLiteralPin)
{
	return SNew(STextBlock).Text(INVTEXT("Test"));
}

TSharedRef<SWidget> SM2LiteralControllerWidget::MakeWidgetForEnumValue(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption));
}

void SM2LiteralControllerWidget::SetValueForLiteralPin(FMetasoundFrontendLiteral& NewValue)
{
	UMetaSoundSourceBuilder* BuilderContext = LiteralPin->ParentVertex->GetSequencerData()->BuilderContext;
	EMetaSoundBuilderResult BuildResult;

	BuilderContext->SetNodeInputDefault(LiteralPin->GetHandle<FMetaSoundBuilderNodeInputHandle>(), NewValue, BuildResult);

}

void SM2LiteralControllerWidget::OnSelectObject(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection == nullptr)
	{
		return;
	}
	auto SelectedObject = Objects.FindByPredicate([NewSelection](UObject* Object) { return Object->GetName() == *NewSelection; });

	if(SelectedObject)
	{
		LiteralObjectValue = *SelectedObject;
	}
	else {
		LiteralObjectValue = nullptr;
	}

	FMetasoundFrontendLiteral NewLiteral;
	NewLiteral.Set(LiteralObjectValue);

	SetValueForLiteralPin(NewLiteral);

}

void SM2LiteralControllerWidget::OnSelectEnum(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection == nullptr)
	{
		return;
	}

	auto NewValue = EnumInterface->FindByName(FName(*NewSelection));
	FMetasoundFrontendLiteral NewLiteral;
	LiteralIntValue = NewValue.GetValue().Value;
	NewLiteral.Set(NewValue.GetValue().Value);

	SetValueForLiteralPin(NewLiteral);
}

void SM2LiteralControllerWidget::OnLiteralValueChanged(float NewValue)
{
	FMetasoundFrontendLiteral NewLiteral;
	LiteralFloatValue = NewValue;
	NewLiteral.Set(NewValue);

	SetValueForLiteralPin(NewLiteral);
}

void SM2LiteralControllerWidget::OnLiteralValueChanged(int32 NewValue)
{
	FMetasoundFrontendLiteral NewLiteral;
	LiteralIntValue = NewValue;
	NewLiteral.Set(NewValue);

	SetValueForLiteralPin(NewLiteral);
}

void SM2LiteralControllerWidget::OnLiteralValueChanged(const FString& NewValue)
{
	FMetasoundFrontendLiteral NewLiteral;
	LiteralStringValue = NewValue;
	NewLiteral.Set(NewValue);

	SetValueForLiteralPin(NewLiteral);
}

void SM2LiteralControllerWidget::OnLiteralValueChanged(ECheckBoxState NewValue)
{
	bool bNewValue = NewValue == ECheckBoxState::Checked;
	
	FMetasoundFrontendLiteral NewLiteral;
	bLiteralBoolValue = bNewValue;
	NewLiteral.Set(bNewValue);

	SetValueForLiteralPin(NewLiteral);
}

FReply SM2LiteralControllerWidget::ExecuteTriggerParameter()
{
	LiteralPin->ParentVertex->GetSequencerData()->ExecuteTriggerParameter(LiteralPin->AutoGeneratedGraphInputName);

	return FReply::Handled();
}

bool SM2LiteralControllerWidget::IsControlEnabled() const
{
	//if we're connected to something we should return disabled

	if (LiteralPin->LinkedPin) return false;
	
	return true;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION