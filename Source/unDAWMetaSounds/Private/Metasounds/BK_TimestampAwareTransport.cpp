// Copyright AmirBK & unDAW, adapted from original sources by Epic Games, Inc.

#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeInterface.h"
#include "MetasoundParamHelper.h"
#include "MetasoundSampleCounter.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundVertex.h"

#include "HarmonixMetasound/Common.h"
#include "HarmonixMetasound/DataTypes/MusicTransport.h"
#include "HarmonixMetasound/DataTypes/MusicTimestamp.h"
#include "HarmonixMetasound/DataTypes/MidiClock.h"
#include "HarmonixMetasound/DataTypes/MusicSeekRequest.h"

#define LOCTEXT_NAMESPACE "unDAWMetasound"

namespace unDAWMetasound
{
	using namespace Metasound;

	namespace TimeStampedTransportVertexNames
	{
		METASOUND_PARAM(TriggerDuringSeek, "Trigger During Seek", "Whether a trigger should be generated is a seek over the timestamp is detected.")
	}

	class FTriggerAndTimestampToTransportOperator : public TExecutableOperator<FTriggerAndTimestampToTransportOperator>, public FMidiPlayCursor
	{
	public:
		static const FNodeClassMetadata& GetNodeInfo();
		static const FVertexInterface& GetVertexInterface();
		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults);

		FTriggerAndTimestampToTransportOperator(const FBuildOperatorParams& InParams,
									const FTriggerReadRef&         InTriggerPrepare,
									const FTriggerReadRef&         InTriggerPlay,
									const FTriggerReadRef&         InTriggerPause, 
									const FTriggerReadRef&         InTriggerContinue, 
									const FTriggerReadRef&         InTriggerStop, 
									const FTriggerReadRef&         InTriggerKill,
									const FTriggerReadRef&         InTriggerSeek,
									const FMusicSeekTargetReadRef& InSeekDestination,
									const FBoolReadRef& InTriggerDuringSeek,
									const FMusicTimestampReadRef& InTimestamp);

		virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override;
		virtual void BindOutputs(FOutputVertexInterfaceData& InVertexData) override;
		virtual FDataReferenceCollection GetInputs() const override;
		virtual FDataReferenceCollection GetOutputs() const override;

		void Reset(const FResetParams& ResetParams);
		
		void Execute();

		void CalculateTriggerTick();

	private:
		//** INPUTS (original stuff)
		FTriggerReadRef PrepareInPin;
		FTriggerReadRef PlayInPin;
		FTriggerReadRef PauseInPin;
		FTriggerReadRef ContinueInPin;
		FTriggerReadRef StopInPin;
		FTriggerReadRef KillInPin;
		FTriggerReadRef TriggerSeekInPin;
		FMusicSeekTargetReadRef SeekDestinationInPin;

		//using namespace HarmonixMetasound;

		//inputs - unDAW additions
		FMusicTimestampReadRef                       TimestampInPin;
		//FEnumMidiClockSubdivisionQuantizationReadRef QuantizeUnitInPin;
		FBoolReadRef		                         TriggerDuringSeekInPin;

		//** OUTPUTS
		HarmonixMetasound::FMusicTransportEventStreamWriteRef TransportOutPin;



		//Midi play cursor thingies 

		 		//** DATA (current state)
		FMusicTimestamp CurrentTimestamp{1, 1.0f};
		EMidiClockSubdivisionQuantization Quantize = EMidiClockSubdivisionQuantization::None;

		int32 TriggerTick = 0;

		struct FTickSpan
		{
			int32 FromTick;
			int32 ThruTick;
			int32 BlockFrameIndex;
			bool  bIsSeek;
			FTickSpan(int32 InFromTick, int32 InThruTick, int32 InBlockFrameIndex, bool InIsSeek)
				: FromTick(InFromTick)
				, ThruTick(InThruTick)
				, BlockFrameIndex(InBlockFrameIndex)
				, bIsSeek(InIsSeek)
			{}
		};
		TArray<FTickSpan> TickSpans;

		//** BEGIN FMidiPlayCursor
		virtual void SeekToTick(int32 Tick) override { SeekThruTick(Tick - 1); }
		virtual void SeekThruTick(int32 Tick) override;
		virtual void AdvanceThruTick(int32 Tick, bool IsPreRoll) override;
		// We have to override this to disambiguate the FMidiPlayCursor Reset and the MS operator Reset
		virtual void Reset(bool ForceNoBroadcast) override { FMidiPlayCursor::Reset(ForceNoBroadcast); }
		//** END FMidiPlayCursor

	};

	class FTriggerToTransportNode : public FNodeFacade
	{
	public:
		FTriggerToTransportNode(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<FTriggerAndTimestampToTransportOperator>())
		{}
		virtual ~FTriggerToTransportNode() override = default;
	};

	METASOUND_REGISTER_NODE(FTriggerToTransportNode)

	const FNodeClassMetadata& FTriggerAndTimestampToTransportOperator::GetNodeInfo()
	{
		auto InitNodeInfo = []() -> FNodeClassMetadata
		{
			FNodeClassMetadata Info;
			Info.ClassName        = { TEXT("unDAW"), TEXT("EnhancedTriggerToTransport"), TEXT("") };
			Info.MajorVersion     = 0;
			Info.MinorVersion     = 2;
			Info.DisplayName      = METASOUND_LOCTEXT("TriggerToTransportNode_DisplayName", "Trigger To Music Transport");
			Info.Description      = METASOUND_LOCTEXT("TriggerToTransportNode_Description", "Combines input triggers into meaningful music transport requests.");
			Info.Author           = TEXT("Amir Ben-Kiki");
			Info.PromptIfMissing  = PluginNodeMissingPrompt;
			Info.DefaultInterface = GetVertexInterface();
			Info.CategoryHierarchy = { INVTEXT("unDAW"), INVTEXT("Music") };
			return Info;
		};

		static const FNodeClassMetadata Info = InitNodeInfo();

		return Info;
	}

	const FVertexInterface& FTriggerAndTimestampToTransportOperator::GetVertexInterface()
	{
		using namespace HarmonixMetasound::CommonPinNames;

		static const FVertexInterface Interface(
			FInputVertexInterface(
				TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::TransportPrepare)),
				TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::TransportPlay)),
				TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::TransportPause)),
				TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::TransportContinue)),
				TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::TransportStop)),
				TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::TransportKill)),
				TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::TriggerSeek)),
				TInputDataVertex<FMusicSeekTarget>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::SeekDestination))

			),
			FOutputVertexInterface(
				TOutputDataVertex<HarmonixMetasound::FMusicTransportEventStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::Transport))
			)
		);

		return Interface;
	}

	TUniquePtr<IOperator> FTriggerAndTimestampToTransportOperator::CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
	{
		using namespace HarmonixMetasound::CommonPinNames;

		const FInputVertexInterfaceData& InputData = InParams.InputData;
		FTriggerReadRef InTriggerPrepare          = InputData.GetOrConstructDataReadReference<FTrigger>(METASOUND_GET_PARAM_NAME(Inputs::TransportPrepare), InParams.OperatorSettings);
		FTriggerReadRef InTriggerPlay             = InputData.GetOrConstructDataReadReference<FTrigger>(METASOUND_GET_PARAM_NAME(Inputs::TransportPlay), InParams.OperatorSettings);
		FTriggerReadRef InTriggerPause            = InputData.GetOrConstructDataReadReference<FTrigger>(METASOUND_GET_PARAM_NAME(Inputs::TransportPause), InParams.OperatorSettings);
		FTriggerReadRef InTriggerContinue         = InputData.GetOrConstructDataReadReference<FTrigger>(METASOUND_GET_PARAM_NAME(Inputs::TransportContinue), InParams.OperatorSettings);
		FTriggerReadRef InTriggerStop             = InputData.GetOrConstructDataReadReference<FTrigger>(METASOUND_GET_PARAM_NAME(Inputs::TransportStop), InParams.OperatorSettings);
		FTriggerReadRef InTriggerKill             = InputData.GetOrConstructDataReadReference<FTrigger>(METASOUND_GET_PARAM_NAME(Inputs::TransportKill), InParams.OperatorSettings);
		FTriggerReadRef InTriggerSeek             = InputData.GetOrConstructDataReadReference<FTrigger>(METASOUND_GET_PARAM_NAME(Inputs::TriggerSeek), InParams.OperatorSettings);
		FMusicSeekTargetReadRef InSeekDestination = InputData.GetOrConstructDataReadReference<FMusicSeekTarget>(METASOUND_GET_PARAM_NAME(Inputs::SeekDestination));
		FBoolReadRef  InTriggerDuringSeek = InputData.GetOrCreateDefaultDataReadReference<bool>(METASOUND_GET_PARAM_NAME(TimeStampedTransportVertexNames::TriggerDuringSeek), InParams.OperatorSettings);


		//FMusicTransportEventStreamReadRef InTransport = InputData.GetOrConstructDataReadReference<FMusicTransportEventStream>(METASOUND_GET_PARAM_NAME(Inputs::Transport), InParams.OperatorSettings);
		FMusicTimestampReadRef InTimestamp = InputData.GetOrConstructDataReadReference<FMusicTimestamp>(METASOUND_GET_PARAM_NAME(Inputs::Timestamp), 1, 1.0f);
		//FMidiClockReadRef InMidiClock = InputData.GetOrConstructDataReadReference<FMidiClock>(METASOUND_GET_PARAM_NAME(Inputs::MidiClock), InParams.OperatorSettings);
		return MakeUnique<FTriggerAndTimestampToTransportOperator>(InParams,
			InTriggerPrepare,
			InTriggerPlay,
			InTriggerPause,
			InTriggerContinue,
			InTriggerStop,
			InTriggerKill,
			InTriggerSeek,
			InSeekDestination,
			InTriggerDuringSeek,
			InTimestamp);
	}

	FTriggerAndTimestampToTransportOperator::FTriggerAndTimestampToTransportOperator(const FBuildOperatorParams& InParams,
															 const FTriggerReadRef&   InTriggerPrepare,
															 const FTriggerReadRef&   InTriggerPlay,
															 const FTriggerReadRef&   InTriggerPause,
															 const FTriggerReadRef&   InTriggerContinue,
															 const FTriggerReadRef&   InTriggerStop,
															 const FTriggerReadRef&   InTriggerKill,
															 const FTriggerReadRef&   InTriggerSeek,
															 const FMusicSeekTargetReadRef& InSeekDestination,
															const FBoolReadRef& InTriggerDuringSeek,
															const FMusicTimestampReadRef& InTimestamp)
		: PrepareInPin(InTriggerPrepare)
		, PlayInPin(InTriggerPlay)
		, PauseInPin(InTriggerPause)
		, ContinueInPin(InTriggerContinue)
		, StopInPin(InTriggerStop)
		, KillInPin(InTriggerKill)
		, TriggerSeekInPin(InTriggerSeek)
		, SeekDestinationInPin(InSeekDestination)
		,TimestampInPin(InTimestamp)
		,TriggerDuringSeekInPin(InTriggerDuringSeek)
		,TransportOutPin(HarmonixMetasound::FMusicTransportEventStreamWriteRef::CreateNew(InParams.OperatorSettings))
	{
		Reset(InParams);
	}

	void FTriggerAndTimestampToTransportOperator::BindInputs(FInputVertexInterfaceData& InVertexData)
	{
		using namespace HarmonixMetasound::CommonPinNames;

		InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::TransportPrepare), PrepareInPin);
		InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::TransportPlay), PlayInPin);
		InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::TransportPause), PauseInPin);
		InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::TransportContinue), ContinueInPin);
		InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::TransportStop),  StopInPin);
		InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::TransportKill), KillInPin);
		InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::TriggerSeek), TriggerSeekInPin);
		InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::SeekDestination), SeekDestinationInPin);
	}

	void FTriggerAndTimestampToTransportOperator::BindOutputs(FOutputVertexInterfaceData& InVertexData)
	{
		using namespace HarmonixMetasound::CommonPinNames;

		InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Outputs::Transport), TransportOutPin);
	}

	FDataReferenceCollection FTriggerAndTimestampToTransportOperator::GetInputs() const
	{
		// This should never be called. Bind(...) is called instead. This method
		// exists as a stop-gap until the API can be deprecated and removed.
		checkNoEntry();
		return {};
	}

	FDataReferenceCollection FTriggerAndTimestampToTransportOperator::GetOutputs() const
	{
		// This should never be called. Bind(...) is called instead. This method
		// exists as a stop-gap until the API can be deprecated and removed.
		checkNoEntry();
		return {};
	}

	void FTriggerAndTimestampToTransportOperator::Reset(const FResetParams& ResetParams)
	{
		TransportOutPin->Reset();
	}

	void FTriggerAndTimestampToTransportOperator::Execute()
	{
		TransportOutPin->Reset();

		// early out if no transport changes are pending...
		if (!PrepareInPin->IsTriggeredInBlock() &&
			!PlayInPin->IsTriggeredInBlock() &&
			!PauseInPin->IsTriggeredInBlock() &&
			!ContinueInPin->IsTriggeredInBlock() &&
			!StopInPin->IsTriggeredInBlock() &&
			!KillInPin->IsTriggeredInBlock() &&
			!TriggerSeekInPin->IsTriggeredInBlock())
		{
			return;
		}

		auto AddEvents = [this](FTriggerReadRef& Pin, HarmonixMetasound::EMusicPlayerTransportRequest Request)
			{
				for (int32 SampleFrame : Pin->GetTriggeredFrames())
				{
					TransportOutPin->AddTransportRequest(Request, SampleFrame);
				}
			};

		// The order here is intentional. It assures that for requests on the exact same sample index...
		// 1 - Stops and Kills will be processed last. This is important to avoid "stuck notes".
		// 2 - Seeks happen before Plays so that we don't "pre-roll" for a play from the beginning 
		//     and then immediately "pre-roll" again to start from the seeked to position.


		// BK: So I'll need to enter the timestamp same as seek??

		for (int32 SampleFrame : TriggerSeekInPin->GetTriggeredFrames())
		{
			TransportOutPin->AddSeekRequest(SampleFrame, *SeekDestinationInPin);
		}
		AddEvents(PrepareInPin, HarmonixMetasound::EMusicPlayerTransportRequest::Prepare);
		AddEvents(PlayInPin, HarmonixMetasound::EMusicPlayerTransportRequest::Play);
		AddEvents(PauseInPin, HarmonixMetasound::EMusicPlayerTransportRequest::Pause);
		AddEvents(ContinueInPin, HarmonixMetasound::EMusicPlayerTransportRequest::Continue);
		AddEvents(StopInPin, HarmonixMetasound::EMusicPlayerTransportRequest::Stop);
		AddEvents(KillInPin, HarmonixMetasound::EMusicPlayerTransportRequest::Kill);
		
	}


	//Midi play cursor methods

	void FTriggerAndTimestampToTransportOperator::CalculateTriggerTick()
	{
		//const FSongMaps& SongMaps = MidiClockInPin->GetSongMaps();
		//TriggerTick = SongMaps.CalculateMidiTick(CurrentTimestamp, Quantize);
	}

		void FTriggerAndTimestampToTransportOperator::SeekThruTick(int32 Tick)
	{
		int32 TickProceedingThisAdvance = CurrentTick;
		FMidiPlayCursor::SeekThruTick(Tick);

		// don't trigger if seeking backward or no progress being made...
		if (Tick < TickProceedingThisAdvance || TickProceedingThisAdvance == CurrentTick)
		{
			return;
		}

		//TickSpans.Emplace(TickProceedingThisAdvance, CurrentTick, MidiClockInPin->GetCurrentBlockFrameIndex(), true);
	}

	void FTriggerAndTimestampToTransportOperator::AdvanceThruTick(int32 Tick, bool IsPreRoll)
	{
		int32 TickProceedingThisAdvance = CurrentTick;
		FMidiPlayCursor::AdvanceThruTick(Tick, IsPreRoll);

		// don't trigger during preroll or if no progress being made...
		if (IsPreRoll || TickProceedingThisAdvance == CurrentTick)
		{
			return;
		}

		//TickSpans.Emplace(TickProceedingThisAdvance, CurrentTick, MidiClockInPin->GetCurrentBlockFrameIndex(), false);
	}


}

#undef LOCTEXT_NAMESPACE // "unDAWMetasound"
