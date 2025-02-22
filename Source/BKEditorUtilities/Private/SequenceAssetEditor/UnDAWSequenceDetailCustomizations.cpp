
#include "UnDAWSequenceDetailCustomizations.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "SMidiTrackControlsWidget.h"
#include "DetailWidgetRow.h"


void FSequenceAssetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Outers;
	DetailBuilder.GetObjectsBeingCustomized(Outers);
	if (Outers.Num() == 0) return;
	SequenceData = Cast<UDAWSequencerData>(Outers[0].Get());

	DetailBuilder.EditCategory("MetasoundAsset")
		.AddCustomRow(FText::FromString("MetasoundAsset"))
		.WholeRowContent()
		[
			SNew(SButton)
				.Text(INVTEXT("Build Asset"))
				.OnClicked_Lambda([this]() { 
				UMetaSoundEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UMetaSoundEditorSubsystem>();
				EMetaSoundBuilderResult Result;
				FString AuthorName = FString("UnDAW");
				FString AssetName;
				SequenceData->GetName(AssetName);
				//append 'Metasound' to the asset name
				AssetName.Append(FString(TEXT("Metasound")));
				FString PackageName = FString(TEXT("/Game/"));
				Subsystem->BuildToAsset(SequenceData->BuilderContext, Subsystem->GetDefaultAuthor(), AssetName, PackageName, Result);
				//print result
				UE_LOG(LogTemp, Warning, TEXT("Build Result: %s"), Result == EMetaSoundBuilderResult::Succeeded ? TEXT("Succeeded") : TEXT("Failed"));


				return FReply::Handled();})
		];

	DetailBuilder.EditCategory("Tracks")
		.AddCustomRow(FText::FromString("Tracks"))
		.WholeRowContent()
		[
			SNew(SBox)
				.MaxDesiredHeight(300)
				[
				SAssignNew(MidiInputTracks, SScrollBox)

				]
		];

	UpdateMidiInputTracks();
	SequenceData->OnMidiDataChanged.AddLambda([&]()
		{
			UpdateMidiInputTracks();
		});

}

FSequenceAssetDetails::~FSequenceAssetDetails()
{
	SequenceData->OnMidiDataChanged.RemoveAll(this);

	MidiInputTracks.Reset();
	NodeDetailsView.Reset();
}

void FSequenceAssetDetails::UpdateMidiInputTracks()
{
	MidiInputTracks->ClearChildren();

	MidiInputTracks->AddSlot()

		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SButton)
						.Text(INVTEXT("Add Track"))
						.OnClicked_Lambda([this]() { SequenceData->AddTrack(); return FReply::Handled(); })
				]

		];

	for (SIZE_T i = 0; i < SequenceData->M2TrackMetadata.Num(); i++)
	{
		MidiInputTracks->AddSlot()
			[
				SNew(SMIDITrackControls)
					.TrackData(&SequenceData->M2TrackMetadata[i])
					.slotInParentID(i)
					//.OnFusionPatchChanged(this, &FSequenceAssetDetails::OnFusionPatchChangedInTrack)
					.ConnectedGraph(SequenceData->M2SoundGraph)
					.SequencerData(SequenceData)

			];
	}
}
