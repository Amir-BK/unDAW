#pragma once

#include "CoreMinimal.h"

#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "M2SoundGraphData.h"

#include "MetasoundEditorSubsystem.h"


class FSequenceAssetDetails : public IDetailCustomization
{
public:
	// This function will be called when the properties are being customized
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	~FSequenceAssetDetails();

	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShareable(new FSequenceAssetDetails()); }
private:
	UDAWSequencerData* SequenceData = nullptr;


	TSharedPtr<SScrollBox> MidiInputTracks;

	void UpdateMidiInputTracks();

	// This function will create a new instance of this class as a shared pointer

	TSharedPtr<IDetailsView> NodeDetailsView;
};