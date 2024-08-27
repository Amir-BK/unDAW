#pragma once

#include "CoreMinimal.h"

#include "MovieSceneNameableTrack.h"

#include "M2SoundGraphData.h"
#include "UndawSequenceMovieSceneTrack.generated.h"



UCLASS()
class BKMUSICCORE_API UUndawSequenceMovieSceneTrack : public UMovieSceneNameableTrack
{
	GENERATED_BODY()

private:

public:

	TObjectPtr<UDAWSequencerData> DAWSequencerData;




	

public:

	virtual UMovieSceneSection* AddNewDAWDataOnRow(UDAWSequencerData* DAWData, FFrameNumber Time, int32 RowIndex);

	//begin UMovieSceneTrack interface

	virtual UMovieSceneSection* CreateNewSection() override;
	virtual void AddSection(UMovieSceneSection& Section) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	virtual bool HasSection(const UMovieSceneSection& Section) const override;
	virtual void RemoveSection(UMovieSceneSection& Section) override;
	virtual void RemoveSectionAt(int32 SectionIndex) override;
	virtual bool IsEmpty() const override;
	virtual const TArray<UMovieSceneSection*>& GetAllSections() const override;
	virtual bool SupportsMultipleRows() const override;

	//END UMovieSceneTrack Interface



	private:
		private:

			/** List of all root audio sections */
			UPROPERTY()
			TArray<TObjectPtr<UMovieSceneSection>> DAWSections;

};
