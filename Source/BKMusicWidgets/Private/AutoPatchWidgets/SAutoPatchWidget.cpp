// Fill out your copyright notice in the Description page of Project Settings.

#include "AutoPatchWidgets/SAutoPatchWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSpacer.h"
#include "unDAWSettings.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SNumericEntryBox.h"

#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "SlateOptMacros.h"
#include "MetasoundBuilderSubsystem.h"
#include "SSearchableComboBox.h"


#include "M2SoundGraphStatics.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAutoPatchWidget::Construct(const FArguments& InArgs, const UM2SoundVertex* InVertex)
{
	Vertex = InVertex;

	ChildSlot
		[
			SAssignNew(ContentBox, SConstraintCanvas)

		];



	//traverse the input pins, if we have a float literal, create a knob for it.

	auto UndawWidgetSettings = GetMutableDefault<UndawWidgetsSettings>();
	auto UndawSettings = GetMutableDefault<UUNDAWSettings>();

	//cache for vertex
	//auto* Settings = GetDefault<UUNDAWSettings>();
	auto* WidgetSettings = GetDefault<UndawWidgetsSettings>();

	//cast parent to patch vertex and get name
	auto* VertexFromPin = Cast<UM2SoundPatch>(Vertex);
	PatchName = VertexFromPin->Patch->GetFName();

	const FName& VertexName = PatchName;

	if (!UndawSettings->Cache.Contains(VertexName))
	{
		FCachedVertexPinInfo NewInfo;
		UndawSettings->Cache.Add(VertexName, NewInfo);
	}


	for (const auto& [Name, Pin] : Vertex->InputM2SoundPins)
	{
	//cache for pin
	

		if (Pin->IsA<UM2MetasoundLiteralPin>())
		{
			if (!UndawSettings->Cache[VertexName].FloatPinConfigs.Contains(Name))
			{
				FM2SoundFloatPinConfig NewConfig;
				NewConfig.MinValue = 0.0f;
				NewConfig.MaxValue = 1.0f;
				NewConfig.GridX = UndawSettings->Cache[VertexName].FloatPinConfigs.Num();
				UndawSettings->Cache[VertexName].FloatPinConfigs.Add(Name, NewConfig);
			}

			Config = &UndawSettings->Cache[VertexName].FloatPinConfigs[Name];
			
			
			auto LiteralPin = Cast<UM2MetasoundLiteralPin>(Pin);
			if (LiteralPin->DataType == "Float")
			{



				ContentBox->AddSlot()
					.AutoSize(true)
					.Alignment(FVector2D(0.0f, 0.0f))
					.Offset(FMargin(Config->GridX * 10.0f, Config->GridY * 10.0f, 0.0f, 0.0f))

					[

						SNew(SMaterialControllerFloatWidget, LiteralPin, Config)
							.ToolTipText(FText::FromName(Name))

					];


				//ContentBox->SetColumnFill(Config->GridX, 1.0f);
				//ContentBox->SetRowFill(Config->GridY, 1.0f);
			}

			if (LiteralPin->DataType == "Bool")
			{
				ContentBox->AddSlot()
					.AutoSize(true)
					.Offset(FMargin(Config->GridX * 10.0f, Config->GridY * 10.0f, 0.0f, 0.0f))
					.Alignment(FVector2D(0.0f, 0.0f))

				//	.Alignment(FVector2D(Config->GridX * 10.0f, Config->GridY * 10.0f))
					[
						SNew(SAudioMaterialButton)
							.ToolTipText(FText::FromName(Name))
							//.OnClicked_Lambda([LiteralPin]() { LiteralPin->ExecuteTriggerParameter(); })
							.AudioMaterialButtonStyle(UndawWidgetSettings->GetButtonStyle())
							.OnBooleanValueChanged_Lambda([LiteralPin](bool NewValue) {
							EMetaSoundBuilderResult BuildResult;
							auto NonConstLiteralPin = const_cast<UM2MetasoundLiteralPin*>(LiteralPin);
							NonConstLiteralPin->LiteralValue.Set(NewValue);
							NonConstLiteralPin->ParentVertex->GetSequencerData()->BuilderContext->SetNodeInputDefault(LiteralPin->GetHandle<FMetaSoundBuilderNodeInputHandle>(), NonConstLiteralPin->LiteralValue, BuildResult);
							
								})

					];

				//ContentBox->SetColumnFill(Config->GridX, 1.0f);
				//ContentBox->SetRowFill(Config->GridY, 1.0f);
			}
		
		
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

	Color = FLinearColor::White;




	//get undaw settings and use colors from there
	auto Settings = UUNDAWSettings::Get();
	TSharedPtr<FString> CurrentSelection = nullptr;
	using namespace Metasound;

	ChildSlot
		[
			SAssignNew(MainHorizontalBox, SHorizontalBox)
		];
	
	//if direction is output, we're done
	if (InLiteralPin.Direction == M2Sound::Pins::EPinDirection::Output)
	{
		return;
	}

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
						//.TextStyle(FAppStyle::Get(), TEXT("Graph.Node.PinName"))
						.OnClicked(this, &SM2LiteralControllerWidget::ExecuteTriggerParameter)
						
				];



			break;
		}

		LiteralPin->LiteralValue.TryGet(bLiteralBoolValue);

		PinColor = Settings->BooleanPinTypeColor;
		MainHorizontalBox->AddSlot()
			.MaxWidth(200)
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
		LiteralPin->LiteralValue.TryGet(LiteralFloatValue);

		MainHorizontalBox->AddSlot()
			.MaxWidth(200)
			[
				SNew(SNumericEntryBox<float>)
					.AllowSpin(true)
					.SpinBoxStyle(FAppStyle::Get(), TEXT("NumericEntrySpinBox"))
					.OnValueChanged(this, &SM2LiteralControllerWidget::OnLiteralValueChanged)
					.Value_Lambda([this]() -> TOptional<float> { return LiteralFloatValue; })
					.MinDesiredValueWidth(100)

					//& FAppStyle::Get().GetWidgetStyle<FSpinBoxStyle>("NumericEntrySpinBox")
					
				//.Value_Lambda([this, &InLiteralPin]() -> TOptional<float> { return InLiteralPin.GetFloatValue(); })
				//.OnValueCommitted_Lambda([this, &InLiteralPin](float NewValue, ETextCommit::Type CommitType) { InLiteralPin.SetFloatValue(NewValue); })
			];

		break;
	case ELiteralType::Integer:
		LiteralPin->LiteralValue.TryGet(LiteralIntValue);

		if (Info.bIsEnum)
		{
			PinColor = Settings->IntPinTypeColor;
			EnumInterface = Metasound::Frontend::IDataTypeRegistry::Get().GetEnumInterfaceForDataType(InLiteralPin.DataType);
			auto DefaultEnumOption = EnumInterface->GetDefaultValue();
			//TArray<TSharedPtr<FString>> EnumOptions;
			auto NameSpaceString = EnumInterface->GetNamespace().ToString() + TEXT("::");
			for (int i = 0; i < EnumInterface->GetAllEntries().Num(); i++)
			{
				//auto Entry = EnumInterface->GetAllEntries()[i];
				auto EntryName = EnumInterface->ToName(i);
				FString EntryString;
				if (EntryName.IsSet())
				{
				EntryString = EntryName->ToString();
				EntryString.RemoveFromStart(NameSpaceString);
				//EnumOptions.Add(MakeShared<FString>(EntryString));
				}
				else {
					EntryString = TEXT("None");
				}

				EnumOptionToValue.Add(MakeShared<FString>(EntryString), i);
				
			}

			EnumOptionToValue.GenerateKeyArray(EnumOptions);

			MainHorizontalBox->AddSlot()
				.MaxWidth(200)
				[
					SNew(SSearchableComboBox)
					.OptionsSource(&EnumOptions)
					.InitiallySelectedItem(MakeShared<FString>(GetEnumValue().ToString()))
					//.ComboBoxStyle(FAppStyle::Get(), TEXT("Graph.Node.PinName"))
					.OnGenerateWidget_Lambda([this](TSharedPtr<FString> InOption) { return MakeWidgetForEnumValue(InOption); })
						[
							SNew(STextBlock)
								.Text_Lambda([this]() -> FText { return GetEnumValue(); })
						]	
					.OnSelectionChanged(this, &SM2LiteralControllerWidget::OnSelectEnum)

					//.OnSelectionChanged_Lambda([this, &InLiteralPin](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo) { InLiteralPin.SetIntValue(EnumInterface->FromName(*NewSelection)); })
				];
		}
		else
		{
			PinColor = Settings->IntPinTypeColor;
			MainHorizontalBox->AddSlot()
				.MaxWidth(200)
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
		if(!LiteralPin->LiteralValue.TryGet(LiteralStringValue))
		{
			LiteralStringValue = TEXT("Couldn't read string value?!?");
		}
		PinColor = Settings->StringPinTypeColor;
		MainHorizontalBox->AddSlot()
			.AutoWidth()
			[
				SNew(SEditableTextBox)
					.Text_Lambda([this]() -> FText { return FText::FromString(LiteralStringValue); })
					//.Style(FAppStyle::Get(), TEXT("Graph.Node.PinName"))
					.OnTextCommitted(this, &SM2LiteralControllerWidget::OnLiteralValueChanged)
					.MinDesiredWidth(300)
				//.Text_Lambda([this, &InLiteralPin]() -> FText { return FText::FromString(InLiteralPin.GetStringValue()); })
				//.OnTextCommitted_Lambda([this, &InLiteralPin](const FText& NewText, ETextCommit::Type CommitType) { InLiteralPin.SetStringValue(NewText.ToString()); })
			];
		break;
	case ELiteralType::UObjectProxy:
		LiteralPin->LiteralValue.TryGet(LiteralObjectValue);

		Objects = UM2SoundGraphStatics::GetAllObjectsOfClass(Info.ProxyGeneratorClass);

		//add none
		UObjectOptions.Add(MakeShared<FString>(TEXT("None")));
		

		for (UObject* Object : Objects)
		{
			auto NewOption = MakeShared<FString>(Object->GetName());

			if (Object == LiteralObjectValue)
			{
				CurrentSelection = NewOption;
			}
			UObjectOptions.Add(NewOption);
		}

		MainHorizontalBox->AddSlot()
			.MaxWidth(200)
			[
				SNew(SSearchableComboBox)
				.OptionsSource(&UObjectOptions)
				.InitiallySelectedItem(CurrentSelection)

				.OnGenerateWidget_Lambda([this](TSharedPtr<FString> InOption) { return MakeWidgetForEnumValue(InOption); })
				[
					SNew(STextBlock)
						.Text_Lambda([this]() -> FText { return FText::FromString(LiteralObjectValue ? LiteralObjectValue->GetName() : TEXT("None")); })
						//.TextStyle(FAppStyle::Get(), TEXT("Graph.Node.PinName"))
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
		ExtractTimeStampFromLiteral();
		MainHorizontalBox->AddSlot()
			[
				SNew(SNumericEntryBox<int32>)
					.AllowSpin(true)
					.OnValueChanged_Lambda([this](int32 NewValue) { OnTimestampChanged(NewValue, 0.0f); })
					.Value_Lambda([this]() -> TOptional<int32> { return LiteralIntValue; })
			];
		
		MainHorizontalBox->AddSlot()
			[
				SNew(SNumericEntryBox<float>)
				.AllowSpin(true)
				.OnValueChanged_Lambda([this](float NewValue) { OnTimestampChanged(INDEX_NONE, NewValue); })
				.Value_Lambda([this]() -> TOptional<float> { return LiteralFloatValue; })
			];

	}

	//SetValueForLiteralPin(LiteralPin->LiteralValue);


}

TSharedRef<SWidget> SM2LiteralControllerWidget::CreateValueWidget(const UM2MetasoundLiteralPin& InLiteralPin)
{
	return SNew(STextBlock).Text(INVTEXT("Test"));
}

TSharedRef<SWidget> SM2LiteralControllerWidget::MakeWidgetForEnumValue(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption));
}

FText SM2LiteralControllerWidget::GetEnumValue() const
{
	auto Key = EnumOptionToValue.FindKey(LiteralIntValue);
	if(Key != nullptr)
	{
		return FText::FromString(*Key->Get());
	}
	else {
		return FText::FromString(TEXT("None"));
	}


}

void SM2LiteralControllerWidget::UpdateValueForLiteralPin()
{
	UMetaSoundSourceBuilder* BuilderContext = LiteralPin->ParentVertex->GetSequencerData()->BuilderContext;
	EMetaSoundBuilderResult BuildResult;

	BuilderContext->SetNodeInputDefault(LiteralPin->GetHandle<FMetaSoundBuilderNodeInputHandle>(), LiteralPin->LiteralValue, BuildResult);

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

		UM2MetasoundLiteralPin* NonConstLiteralPin = const_cast<UM2MetasoundLiteralPin*>(this->LiteralPin);
		NonConstLiteralPin->LiteralValue.Set(LiteralObjectValue);

	UpdateValueForLiteralPin();

}

void SM2LiteralControllerWidget::OnSelectEnum(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection == nullptr)
	{
		return;
	}
	auto NewValueIndex = EnumOptionToValue.Find(NewSelection);

	//auto NewValue = EnumInterface->FindByName(FName(*NewSelection));
	auto NewValue = EnumInterface->FindByValue(*NewValueIndex);
	LiteralIntValue = *NewValueIndex;
	UM2MetasoundLiteralPin* NonConstLiteralPin = const_cast<UM2MetasoundLiteralPin*>(this->LiteralPin);
	if(NewValue.IsSet())
	{
		NonConstLiteralPin->LiteralValue.Set(LiteralIntValue);
	}
	else {
		NonConstLiteralPin->LiteralValue.Set(EnumInterface->GetDefaultValue());
	}

	UpdateValueForLiteralPin();
}

void SM2LiteralControllerWidget::OnLiteralValueChanged(float NewValue)
{

	LiteralFloatValue = NewValue;
	UM2MetasoundLiteralPin* NonConstLiteralPin = const_cast<UM2MetasoundLiteralPin*>(this->LiteralPin);
	NonConstLiteralPin->LiteralValue.Set(LiteralFloatValue);

	UpdateValueForLiteralPin();
}

void SM2LiteralControllerWidget::OnLiteralValueChanged(int32 NewValue)
{

	LiteralIntValue = NewValue;
	UM2MetasoundLiteralPin* NonConstLiteralPin = const_cast<UM2MetasoundLiteralPin*>(this->LiteralPin);
	NonConstLiteralPin->LiteralValue.Set(LiteralIntValue);


	UpdateValueForLiteralPin();
}

void SM2LiteralControllerWidget::OnLiteralValueChanged(const FString& NewValue)
{
	LiteralStringValue = NewValue;
	UM2MetasoundLiteralPin* NonConstLiteralPin = const_cast<UM2MetasoundLiteralPin*>(this->LiteralPin);
	NonConstLiteralPin->LiteralValue.Set(LiteralStringValue);

	UpdateValueForLiteralPin();
}

void SM2LiteralControllerWidget::OnLiteralValueChanged(const FText& NewText, ETextCommit::Type CommitInfo)
{
	OnLiteralValueChanged(NewText.ToString());
}

void SM2LiteralControllerWidget::OnLiteralValueChanged(ECheckBoxState NewValue)
{
	bool bNewValue = NewValue == ECheckBoxState::Checked;
	
	bLiteralBoolValue = bNewValue;
	UM2MetasoundLiteralPin* NonConstLiteralPin = const_cast<UM2MetasoundLiteralPin*>(this->LiteralPin);
	NonConstLiteralPin->LiteralValue.Set(bLiteralBoolValue);

	UpdateValueForLiteralPin();
}

void SM2LiteralControllerWidget::OnTimestampChanged(int32 bar, float beat)
{
	if(bar != INDEX_NONE)
	{
		LiteralIntValue = bar;
	}

	if(beat != 0.0f)
	{
		LiteralFloatValue = beat;
	}

	auto NewTimeStamp = FMusicTimestamp(LiteralIntValue, LiteralFloatValue);
	//Metasound::Frontend::
	//auto TheLiteral = Metasound::FLiteral(NewTimeStamp);

	UM2MetasoundLiteralPin* NonConstLiteralPin = const_cast<UM2MetasoundLiteralPin*>(this->LiteralPin);
	//NonConstLiteralPin->LiteralValue.SetFromLiteral(NewTimeStamp);
	const auto& SequencerData = LiteralPin->ParentVertex->GetSequencerData();
	NonConstLiteralPin->Timestamp.Bar = LiteralIntValue;
	NonConstLiteralPin->Timestamp.Beat = LiteralFloatValue;

	//UMetasoundParameterPack* NewPack = NewObject<UMetasoundParameterPack>(SequencerData);
	SequencerData->StructParametersPack->SetParameter(LiteralPin->AutoGeneratedGraphInputName, FName("MusicTimeStamp"), NewTimeStamp, false);
	SequencerData->ApplyParameterPack();
	UE_LOG(LogTemp, Warning, TEXT("Bar: %d, Beat: %f"), LiteralIntValue, LiteralFloatValue);
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

void SM2LiteralControllerWidget::ExtractTimeStampFromLiteral()
{
	//we'll need a parameter pack
	auto NewPack = LiteralPin->ParentVertex->GetSequencerData()->StructParametersPack;
	ESetParamResult Result;
	FMusicTimestamp NewTimeStamp =	NewPack->GetParameter<FMusicTimestamp>(LiteralPin->AutoGeneratedGraphInputName, FName("MusicTimeStamp"), Result);
	LiteralIntValue = NewTimeStamp.Bar;
	LiteralFloatValue = NewTimeStamp.Beat;

	UE_LOG(LogTemp, Warning, TEXT("Extract TimeStamp Bar: %d, Beat: %f"), NewTimeStamp.Bar, NewTimeStamp.Beat);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION