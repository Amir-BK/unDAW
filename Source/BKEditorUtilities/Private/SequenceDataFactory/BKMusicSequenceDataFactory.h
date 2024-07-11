// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
//#include "UObject/Object.h"
#include "AssetTypeActions_Base.h"
#include "../SequenceAssetEditor/UnDawSequenceEditorToolkit.h"
#include "M2SoundGraphData.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "AssetRegistry/AssetData.h"
#include <UnDAWPreviewHelperSubsystem.h>
#include "BKMusicSequenceDataFactory.generated.h"


class FDAWSequenceAssetActions : public FAssetTypeActions_Base
{
public:


	static bool IsSessionPreviewPlaying(const FAssetData& AssetData)
	{
		const UAudioComponent* PreviewComp = GEditor->GetPreviewAudioComponent();
		if (PreviewComp && PreviewComp->Sound && PreviewComp->IsPlaying())
		{
			if (PreviewComp->Sound->GetFName() == AssetData.AssetName)
			{
				if (PreviewComp->Sound->GetOutermost()->GetFName() == AssetData.PackageName)
				{
					return true;
				}
			}
		}

		return false;
	}

	UClass* GetSupportedClass() const override
	{
		return UDAWSequencerData::StaticClass();
	}
	FText GetName() const override
	{
		return INVTEXT("unDAW Session Data");
	}
	FColor GetTypeColor() const override 
	{
		return FColor::Purple;
	}
	uint32 GetCategories() override
	{
		return EAssetTypeCategories::Sounds;
	}

	void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor) override
	{
		MakeShared<FUnDAWSequenceEditorToolkit>()->InitEditor(InObjects);
	}


	TSharedPtr<SWidget> GetDawSequenceThumbnailOverlay(const FAssetData& InAssetData, TUniqueFunction<FReply()>&& OnClickedLambdaOverride) const
	{
		auto OnGetDisplayBrushLambda = [InAssetData]() -> const FSlateBrush*
			{
				auto SequenceData = Cast<UDAWSequencerData>(InAssetData.GetAsset());
				//auto SequenceData = Cast<UDAWSequencerData>(InAssetData.GetAsset());

				if (SequenceData && SequenceData->PlayState == TransportPlaying)
				{
					return FAppStyle::GetBrush("MediaAsset.AssetActions.Stop.Large");
				}

				return FAppStyle::GetBrush("MediaAsset.AssetActions.Play.Large");
			};

		auto OnClickedLambda = [InAssetData]() -> FReply
			{
				auto SequenceData = Cast<UDAWSequencerData>(InAssetData.GetAsset());

				if (SequenceData && SequenceData->PlayState == TransportPlaying)
				{
					//unDAW::PreviewPlayback::StopSound();
					SequenceData->SendTransportCommand(EBKTransportCommands::Stop);
				}
				else
				{
					// Load and play sound
					auto PreviewHelper = GEditor->GetEditorSubsystem<UUnDAWPreviewHelperSubsystem>();
					//PreviewHelper->OnDAWPerformerReady

					if (SequenceData && SequenceData->PlayState == ReadyToPlay)
					{
						SequenceData->SendTransportCommand(EBKTransportCommands::Play);
						//SequenceData->MetasoundBuilderHelper->AuditionComponentRef->SetTriggerParameter(FName("unDAW.Transport.Play"));
						return FReply::Handled();
					}
					PreviewHelper->CreateAndPrimePreviewBuilderForDawSequence(SequenceData);
					SequenceData->SendTransportCommand(EBKTransportCommands::Play);


				}
				return FReply::Handled();
			};

		auto OnToolTipTextLambda = [InAssetData]() -> FText
			{
				if (FDAWSequenceAssetActions::IsSessionPreviewPlaying(InAssetData))
				{
					return INVTEXT("Stop selected unDAW Sequence");
				}

				return INVTEXT("Preview the selected unDAW Sequence");
			};

		TSharedPtr<SBox> Box;
		SAssignNew(Box, SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2));

		auto OnGetVisibilityLambda = [Box, InAssetData]() -> EVisibility
			{
				if (Box.IsValid() && (Box->IsHovered() || FDAWSequenceAssetActions::IsSessionPreviewPlaying(InAssetData)))
				{
					return EVisibility::Visible;
				}

				return EVisibility::Hidden;
			};

		TSharedPtr<SButton> Widget;
		SAssignNew(Widget, SButton)
			.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
			.ToolTipText_Lambda(OnToolTipTextLambda)
			.Cursor(EMouseCursor::Default) // The outer widget can specify a DragHand cursor, so we need to override that here
			.ForegroundColor(FSlateColor::UseForeground())
			.IsFocusable(false)
			.OnClicked_Lambda(OnClickedLambda)
			.Visibility_Lambda(OnGetVisibilityLambda)
			[
				SNew(SImage)
					.Image_Lambda(OnGetDisplayBrushLambda)
			];

		Box->SetContent(Widget.ToSharedRef());
		Box->SetVisibility(EVisibility::Visible);

		return Box;
	}

	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& InAssetData) const override;
};


/**
 * 
 */
UCLASS()
class BK_EDITORUTILITIES_API UBKMusicSequenceDataFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	//~ UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;



	virtual bool ShouldShowInNewMenu() const override;

	UBKMusicSequenceDataFactory();

	

};
