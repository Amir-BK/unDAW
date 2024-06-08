// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SequencerData.h"
#include "SGraphNode.h"
#include "M2SoundGraphStatics.h"
#include "M2SoundEdGraphSchema.h"

/**
 * I'm lazy so this slate widget is going to only be used for patches/inserts
 */
template<typename T>
class BK_EDITORUTILITIES_API SM2SoundPatchContainerGraphNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SM2SoundPatchContainerGraphNode)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UEdGraphNode* InGraphNode)
	{
		GraphNode = InGraphNode;

		//Cast graph node to our M2Sound and extract the patch vertex
		UM2SoundPatchContainerNode* AsM2SoundPatchContainerNode = Cast<UM2SoundPatchContainerNode>(InGraphNode);

		//we may not have a patch vertex yet... though really we should prevent that from happening
		if (AsM2SoundPatchContainerNode->Vertex)
		{
			PatchVertex = Cast<UM2SoundPatch>(AsM2SoundPatchContainerNode->Vertex);
			if (PatchVertex->Patch)
			{
				auto PatchName = PatchVertex->Patch->GetDocumentChecked().Metadata.Version.Name;
				SelectedPatch = MakeShareable(new FString(PatchName.ToString()));

			}
		}


		
		Audio::FParameterInterface Interface = T();

		auto InterfaceAsFrontEndVersion = Interface.GetName();

		//print version for debugging
		UE_LOG(LogTemp, Warning, TEXT("Interface Version: %s"), *InterfaceAsFrontEndVersion.ToString());

		const FMetasoundFrontendVersion Version{ Interface.GetName(), { Interface.GetVersion().Major, Interface.GetVersion().Minor } };

		//populate patch options with all metasound patches, get all assets of type cast to patch
		//we will create this by demand when less lazy, for now lets test
		TArray<UMetaSoundPatch*> PatchAssets;
		UM2SoundGraphStatics::GetObjectsOfClass<UMetaSoundPatch>(PatchAssets);

		for (auto Patch : PatchAssets)
		{
			bool bImplementsInterface = Patch->GetDocumentChecked().Interfaces.Contains(Version);
			if (!bImplementsInterface)
			{
				continue;
			}
			auto PatchName = Patch->GetName();
			auto DisplayName = Patch->GetDisplayName();
			auto FormattedName = FString::Printf(TEXT("%s - %s"), *PatchName, *DisplayName.ToString());
			PatchOptions.Add(MakeShareable(new FString(FormattedName)));//  GetDocumentChecked(). Version.Name.ToString())));
		}





		/*
		ChildSlot
		[
			// Populate the widget
		];
		*/
		UpdateGraphNode();
		//SGraphNode(InArgs, InGraphNode));
	}

private:
	T ParameterInterface;


	//Begin SWidget interface
	//set desired size of the widget
	//virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(500, 200); }


	//SGraphNode interface

	TSharedRef<SWidget> CreateNodeContentArea() override
	{

		UE_LOG(LogTemp, Warning, TEXT("SSM2SoundEdGraphNode::CreateNodeContentArea"));

		// NODE CONTENT AREA
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("NoBorder"))
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)

			.Padding(FMargin(0, 3))
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.FillWidth(1.0f)
					[
						// LEFT
						SAssignNew(LeftNodeBox, SVerticalBox)
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Center)
					.AutoWidth()

					[
						// Center
						SAssignNew(MainVerticalBox, SVerticalBox)
							+ SVerticalBox::Slot()

							//patch select

							.AutoHeight()
							[
								SNew(SComboBox<TSharedPtr<FString>>)
									.OptionsSource(&PatchOptions)
									.OnGenerateWidget(this, &SM2SoundPatchContainerGraphNode::MakePatchComboWidget)
									.OnSelectionChanged(this, &SM2SoundPatchContainerGraphNode::OnPatchSelected)
									[
										SNew(STextBlock)
											.Text(this, &SM2SoundPatchContainerGraphNode::GetSelectedPatchName)
									]
							]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					[
						// RIGHT
						SAssignNew(RightNodeBox, SVerticalBox)
					]
			];
	}

	protected:
		TSharedPtr<SVerticalBox> MainVerticalBox;


		TArray<TSharedPtr<FString>> PatchOptions;
		TSharedPtr<FString> SelectedPatch;


		TSharedRef<SWidget> MakePatchComboWidget(TSharedPtr<FString> InItem)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(*InItem));

		}

		void OnPatchSelected(TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo)
		{
			SelectedPatch = InItem;

		}

		FText GetSelectedPatchName() const
		{
			if (SelectedPatch.IsValid())
			{
				return FText::FromString(*SelectedPatch);
			}
			else
			{
				return FText::FromString(TEXT("No Patch Selected"));
			}

		}

		//patch vertex
		UM2SoundPatch* PatchVertex;
};
