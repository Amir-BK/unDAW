// Fill out your copyright notice in the Description page of Project Settings.

#include "AutoPatchWidgets/SAutoPatchWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSpacer.h"
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
	ChildSlot
	[
		SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
					.Text(FText::FromName(InLiteralPin.Name))
					.ColorAndOpacity(FLinearColor::Blue)
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
					.ColorAndOpacity(FLinearColor::Green)
			]
	];
}



END_SLATE_FUNCTION_BUILD_OPTIMIZATION