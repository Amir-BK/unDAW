// Fill out your copyright notice in the Description page of Project Settings.

#include "AutoPatchWidgets/SAutoPatchWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSpacer.h"
#include "unDAWSettings.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAutoPatchWidget::Construct(const FArguments& InArgs, const UM2SoundVertex* InVertex)
{
	Vertex = InVertex;
	
	ChildSlot
	[
		SAssignNew(ContentBox, SVerticalBox)
	];

	for(const auto& [Name,Pin] : Vertex->InputM2SoundPins)
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

	switch(Info.PreferredLiteralType)
	{
		case ELiteralType::Boolean:
		Color = Settings->BooleanPinTypeColor;
		break;
	
		case ELiteralType::Float:
		Color = Settings->FloatPinTypeColor;
		break;
		case ELiteralType::Integer:
		if(Info.bIsEnum)
		{
			Color = Settings->IntPinTypeColor;
		}
		else
		{
			Color = Settings->IntPinTypeColor;
		}

		break;
		case ELiteralType::String:
		Color = Settings->StringPinTypeColor;
		break;
		case ELiteralType::UObjectProxy:
		Color = Settings->ObjectPinTypeColor;
		break;

		default:
			break;
	}

	ChildSlot
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
						.Text(FText::FromName(InLiteralPin.Name))
						.ColorAndOpacity(Color)
				]
				//a spacer
				+ SHorizontalBox::Slot()
				[
					SNew(SSpacer)
						.Size(FVector2D(10, 0))
				]

				//the data type please
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
						.Text(FText::FromName(InLiteralPin.DataType))
						.ColorAndOpacity(Color)
				]
		];
	//make default 


}



END_SLATE_FUNCTION_BUILD_OPTIMIZATION