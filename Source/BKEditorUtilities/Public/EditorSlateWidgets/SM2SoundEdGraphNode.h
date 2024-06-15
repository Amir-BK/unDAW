// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "M2SoundGraphData.h"
#include "SGraphNode.h"
#include "M2SoundGraphStatics.h"
#include "SAudioRadialSlider.h"
#include "Widgets/Layout/SWrapBox.h"
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

	//probably a little expensive but we only use this with editor now, so, we'll see.
	void RegenPatchOptions()
	{
		PatchOptions.Empty();
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
			PatchOptions.Add(MakeShareable(new FString(PatchName)));//  GetDocumentChecked(). Version.Name.ToString())));
		}
	}

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
				auto PatchName = PatchVertex->Patch->GetName();
				SelectedPatch = MakeShareable(new FString(PatchName));
			}
		}

		Audio::FParameterInterface Interface = T();

		auto InterfaceAsFrontEndVersion = Interface.GetName();

		Version = { Interface.GetName(), { Interface.GetVersion().Major, Interface.GetVersion().Minor } };

		RegenPatchOptions();
		AsM2SoundPatchContainerNode->OnNodeUpdated.BindRaw(this, &SM2SoundPatchContainerGraphNode::UpdateAudioKnobs);
		UpdateGraphNode();
	}

private:
	T ParameterInterface;

	//Begin SWidget interface
	//set desired size of the widget
	//virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(500, 200); }

	//SGraphNode interface

	TSharedRef<SWidget> CreateNodeContentArea() override
	{
		
		SAssignNew(MainAudioKnobsBox, SWrapBox)
			.FlowDirectionPreference(EFlowDirectionPreference::LeftToRight)
			.Orientation(EOrientation::Orient_Horizontal)
			.UseAllottedSize(true)
			.PreferredSize(200);


		UpdateAudioKnobs();
		
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
									.OnComboBoxOpening(this, &SM2SoundPatchContainerGraphNode::RegenPatchOptions)
									.OnGenerateWidget(this, &SM2SoundPatchContainerGraphNode::MakePatchComboWidget)
									.OnSelectionChanged(this, &SM2SoundPatchContainerGraphNode::OnPatchSelected)
									[
										SNew(STextBlock)
											.Text(this, &SM2SoundPatchContainerGraphNode::GetSelectedPatchName)
									]
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								MainAudioKnobsBox.ToSharedRef()
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

	void UpdateAudioKnobs()
	{
		if (!MainAudioKnobsBox.IsValid()) return;
		MainAudioKnobsBox->ClearChildren();
		if (PatchVertex)
		{
			for (auto& [Name, Pin] : PatchVertex->InPinsNew)
			{
				if (Pin.DisplayFlags & static_cast<uint8>(EM2SoundPinDisplayFlags::ShowInGraph) && Pin.DataType == "float")
				{
					MainAudioKnobsBox->AddSlot()

						[
							SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(FText::FromString(Name.ToString()))
										
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SAudioRadialSlider)
										.SliderValue(1.0f)
										.ToolTipText(FText::FromString(Name.ToString()))
										.AccessibleText(FText::FromString(Name.ToString()))

								]

						];
				};

			}
		}

	}

protected:
	TSharedPtr<SVerticalBox> MainVerticalBox;
	TSharedPtr<SWrapBox> MainAudioKnobsBox;


	TArray<TSharedPtr<FString>> PatchOptions;
	TSharedPtr<FString> SelectedPatch;
	FMetasoundFrontendVersion Version;

	TSharedRef<SWidget> MakePatchComboWidget(TSharedPtr<FString> InItem)
	{
		return SNew(STextBlock)
			.Text(FText::FromString(*InItem));
	}

	void OnPatchSelected(TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo)
	{
		if(InItem == nullptr)
		{
			return;
		}
		SelectedPatch = InItem;
		//find the patch asset, assign it to the vertex and call update
		UMetaSoundPatch* Patch = UM2SoundGraphStatics::GetPatchByName(*SelectedPatch);
		if (Patch)
		{
			PatchVertex->Patch = Patch;
			PatchVertex->VertexNeedsBuilderUpdates();
		}
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
