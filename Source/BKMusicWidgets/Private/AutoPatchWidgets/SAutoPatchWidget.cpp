// Fill out your copyright notice in the Description page of Project Settings.

#include "AutoPatchWidgets/SAutoPatchWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSpacer.h"
#include "unDAWSettings.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "SlateOptMacros.h"

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

	Metasound::Frontend::IDataTypeRegistry::Get().GetDataTypeInfo(InLiteralPin.DataType, Info);
	//MetaSoundDataRegistry.;

	FLinearColor Color = FLinearColor::White;

	// objects will be blue for now
	if (Info.ProxyGeneratorClass)
	{
		//print proxy generator class name
		UE_LOG(LogTemp, Warning, TEXT("Proxy Generator Class Name: %s"), *Info.ProxyGeneratorClass->GetName());
	}

	//get undaw settings and use colors from there
	auto Settings = UUNDAWSettings::Get();

	using namespace Metasound;

	ChildSlot
		[
			SAssignNew(MainHorizontalBox, SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
						.Text(FText::FromName(InLiteralPin.Name))
						.ColorAndOpacity_Lambda([this]() -> FSlateColor { return PinColor; })
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
		PinColor = Settings->BooleanPinTypeColor;
		MainHorizontalBox->AddSlot()
			[
				SNew(SCheckBox)
			//	.IsChecked_Lambda([this, &InLiteralPin]() -> ECheckBoxState { return InLiteralPin.GetBoolValue() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				//.OnCheckStateChanged_Lambda([this, &InLiteralPin](ECheckBoxState NewState) { InLiteralPin.SetBoolValue(NewState == ECheckBoxState::Checked); })
			];
		break;

	case ELiteralType::Float:
		PinColor = Settings->FloatPinTypeColor;
		MainHorizontalBox->AddSlot()
			[
				SNew(SNumericEntryBox<float>)
				//.Value_Lambda([this, &InLiteralPin]() -> TOptional<float> { return InLiteralPin.GetFloatValue(); })
				//.OnValueCommitted_Lambda([this, &InLiteralPin](float NewValue, ETextCommit::Type CommitType) { InLiteralPin.SetFloatValue(NewValue); })
			];

		break;
	case ELiteralType::Integer:
		if (Info.bIsEnum)
		{
			PinColor = Settings->IntPinTypeColor;
			auto EnumInterface = Metasound::Frontend::IDataTypeRegistry::Get().GetEnumInterfaceForDataType(InLiteralPin.DataType);
			//TArray<TSharedPtr<FString>> EnumOptions;
			for (int i = 0; i < EnumInterface->GetAllEntries().Num(); i++)
			{
				EnumOptions.Add(MakeShared<FString>(EnumInterface->ToName(i)->ToString()));
				UE_LOG(LogTemp, Warning, TEXT("Enum Option: %s"), *EnumInterface->ToName(i)->ToString());
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
							.Text_Lambda([this]() -> FText { return FText::FromString(TEXT("Poop")); })
						]	
				

					//.OnSelectionChanged_Lambda([this, &InLiteralPin](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo) { InLiteralPin.SetIntValue(EnumInterface->FromName(*NewSelection)); })
				];
		}
		else
		{
			PinColor = Settings->IntPinTypeColor;
			MainHorizontalBox->AddSlot()
				[
					SNew(SNumericEntryBox<int32>)
					//.Value_Lambda([this, &InLiteralPin]() -> TOptional<int32> { return InLiteralPin.GetIntValue(); })
					//.OnValueCommitted_Lambda([this, &InLiteralPin](int32 NewValue, ETextCommit::Type CommitType) { InLiteralPin.SetIntValue(NewValue); })
				];
		}

		break;
	case ELiteralType::String:
		PinColor = Settings->StringPinTypeColor;
		break;
	case ELiteralType::UObjectProxy:
		PinColor = Settings->ObjectPinTypeColor;
		break;

	default:
		break;
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

END_SLATE_FUNCTION_BUILD_OPTIMIZATION