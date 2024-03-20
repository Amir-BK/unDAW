// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeInterface.h"
#include "MetasoundParamHelper.h"
#include "MetasoundSampleCounter.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundVertex.h"
#include "HarmonixMetasound/Common.h"
#include "HarmonixMetasound/DataTypes/MidiClock.h"
#include "HarmonixMetasound/DataTypes/MusicTransport.h"
#include "HarmonixMetasound/DataTypes/MusicTimeStamp.h"
#include "MetasoundNodeInterface.h"
//#include "HarmonixMetasound/DataTypes"

#define LOCTEXT_NAMESPACE "unDAWMetaSound"

namespace unDAWMetasound
{
	using namespace Metasound;

	namespace TimeStampedTransportWavePlayerControllerVertexNames
	{
		METASOUND_PARAM(OutputStartTime, "Start Time", "Time into the wave asset to start (seek) the wave asset.")
	}

	class FTimeStampedTransportWavePlayerControllerOperator : public TExecutableOperator<FTimeStampedTransportWavePlayerControllerOperator>, public HarmonixMetasound::FMusicTransportControllable, public FMidiPlayCursor
	{
	public:
		FTimeStampedTransportWavePlayerControllerOperator(const FOperatorSettings& InSettings,
									const HarmonixMetasound::FMusicTransportEventStreamReadRef& InTransport,
									const HarmonixMetasound::FMidiClockReadRef& InMidiClock,
									const FMusicTimestampReadRef& InTimestamp ) :
			HarmonixMetasound::FMusicTransportControllable(HarmonixMetasound::EMusicPlayerTransportState::Prepared),
			TransportInPin(InTransport),
			MidiClockInPin(InMidiClock),
			PlayOutPin(FTriggerWriteRef::CreateNew(InSettings)),
			StopOutPin(FTriggerWriteRef::CreateNew(InSettings)),
			StartTimeOutPin(FTimeWriteRef::CreateNew()),
			BlockSizeFrames(InSettings.GetNumFramesPerBlock()),
			TimestampInPin(InTimestamp),
			SampleRate(InSettings.GetSampleRate())
		{
		}

		static const FVertexInterface& GetVertexInterface()
		{
			using namespace HarmonixMetasound::CommonPinNames;
			using namespace TimeStampedTransportWavePlayerControllerVertexNames;

			static const FVertexInterface Interface(
				FInputVertexInterface(
					TInputDataVertex<HarmonixMetasound::FMusicTransportEventStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Transport)),
					TInputDataVertex<FMusicTimestamp>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Timestamp)),
					TInputDataVertex<HarmonixMetasound::FMidiClock>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiClock))
				),
				FOutputVertexInterface(
					TOutputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::TransportPlay)),
					TOutputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::TransportStop)),
					TOutputDataVertex<FTime>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputStartTime))
				)
			);

			return Interface;
		}

		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto InitNodeInfo = []() -> FNodeClassMetadata
			{
				FNodeClassMetadata Info;
				Info.ClassName = { TEXT("unDAW"), TEXT("TimeStampedTransportWavePlayerController"), TEXT("")};
				Info.MajorVersion = 0;
				Info.MinorVersion = 1;
				Info.DisplayName = INVTEXT("unDAW TimeStamped Music Transport Wave Player Controller");
				Info.Description = INVTEXT("An interface between a music transport and a wave player, and a timestamp!");
				Info.Author = PluginAuthor;
				Info.PromptIfMissing = PluginNodeMissingPrompt;
				Info.DefaultInterface = GetVertexInterface();
				Info.CategoryHierarchy = { INVTEXT("unDAW"), INVTEXT("Music") };
				return Info;
			};

			static const FNodeClassMetadata Info = InitNodeInfo();

			return Info;
		}

		virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override
		{
			using namespace HarmonixMetasound::CommonPinNames;

			InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::Transport), TransportInPin);
			InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::MidiClock), MidiClockInPin);
			InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Inputs::Timestamp), TimestampInPin);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InVertexData) override
		{
			using namespace HarmonixMetasound::CommonPinNames;
			using namespace TimeStampedTransportWavePlayerControllerVertexNames;

			InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Outputs::TransportPlay), PlayOutPin);
			InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(Outputs::TransportStop), StopOutPin);
			InVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputStartTime), StartTimeOutPin);
		}
		
		virtual FDataReferenceCollection GetInputs() const override
		{
			// This should never be called. Bind(...) is called instead. This method
			// exists as a stop-gap until the API can be deprecated and removed.
			checkNoEntry();
			return {};
		}

		virtual FDataReferenceCollection GetOutputs() const override
		{
			// This should never be called. Bind(...) is called instead. This method
			// exists as a stop-gap until the API can be deprecated and removed.
			checkNoEntry();
			return {};
		}

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace HarmonixMetasound;
			using namespace HarmonixMetasound::CommonPinNames;

			const FInputVertexInterfaceData& InputData = InParams.InputData;
			FMusicTransportEventStreamReadRef InTransport = InputData.GetOrConstructDataReadReference<FMusicTransportEventStream>(METASOUND_GET_PARAM_NAME(Inputs::Transport), InParams.OperatorSettings);
			FMusicTimestampReadRef InTimestamp = InputData.GetOrConstructDataReadReference<FMusicTimestamp>(METASOUND_GET_PARAM_NAME(Inputs::Timestamp), 1, 1.0f);
			FMidiClockReadRef InMidiClock = InputData.GetOrConstructDataReadReference<FMidiClock>(METASOUND_GET_PARAM_NAME(Inputs::MidiClock), InParams.OperatorSettings);
			
			return MakeUnique<FTimeStampedTransportWavePlayerControllerOperator>(InParams.OperatorSettings, InTransport, InMidiClock, InTimestamp);
		}

		void Execute()
		{
			using namespace HarmonixMetasound;
			// advance the outputs
			PlayOutPin->AdvanceBlock();
			StopOutPin->AdvanceBlock();

			TransportSpanProcessor TransportHandler = [this](int32 StartFrameIndex, int32 EndFrameIndex, EMusicPlayerTransportState CurrentState)
			{
				switch (CurrentState)
				{
				case EMusicPlayerTransportState::Invalid:
				case EMusicPlayerTransportState::Preparing:
					return EMusicPlayerTransportState::Prepared;

				case EMusicPlayerTransportState::Prepared:
					return EMusicPlayerTransportState::Prepared;

				case EMusicPlayerTransportState::Starting:
					if (!ReceivedSeekWhileStopped())
					{
						// Play from the beginning if we haven't received a seek call while we were stopped...
						*StartTimeOutPin = FTime();
					}
					PlayOutPin->TriggerFrame(StartFrameIndex);
					bPlaying = true;
					return EMusicPlayerTransportState::Playing;

				case EMusicPlayerTransportState::Playing:
					return EMusicPlayerTransportState::Playing;

				case EMusicPlayerTransportState::Seeking:
					if (ReceivedSeekWhileStopped())
					{
						// Assumes the MidiClock is stopped for the remainder of the block.
						*StartTimeOutPin = FTime(MidiClockInPin->GetCurrentHiResMs() * 0.001f);
					}
					else
					{
						StopOutPin->TriggerFrame(StartFrameIndex);
						int32 PlayFrameIndex = FMath::Min(StartFrameIndex + 1, EndFrameIndex);

						// Assumes the MidiClock is playing for the remainder of the block.
						*StartTimeOutPin = FTime(MidiClockInPin->GetCurrentHiResMs() * 0.001f - (BlockSizeFrames - PlayFrameIndex) / SampleRate);
						PlayOutPin->TriggerFrame(PlayFrameIndex);
					}
					// Here we will return that we want to be in the same state we were in before this request to 
					// seek since we can seek "instantaneously"...
					return GetTransportState();

				case EMusicPlayerTransportState::Continuing:
					// Assumes the StartTimeOutPin won't change for the remainder of the block.
					PlayOutPin->TriggerFrame(StartFrameIndex);
					bPlaying = true;
					return EMusicPlayerTransportState::Playing;

				case EMusicPlayerTransportState::Pausing:
					bPlaying = false;
					StopOutPin->TriggerFrame(StartFrameIndex);

					// Assumes the MidiClock is paused for the remainder of the block.
					*StartTimeOutPin = FTime(MidiClockInPin->GetCurrentHiResMs() * 0.001f);
					return EMusicPlayerTransportState::Paused;

				case EMusicPlayerTransportState::Paused:
					return EMusicPlayerTransportState::Paused;

				case EMusicPlayerTransportState::Stopping:
				case EMusicPlayerTransportState::Killing:
					if (bPlaying)
					{
						bPlaying = false;
						StopOutPin->TriggerFrame(StartFrameIndex);
					}
					*StartTimeOutPin = FTime();
					return EMusicPlayerTransportState::Prepared;

				default:
					checkNoEntry();
					return EMusicPlayerTransportState::Invalid;
				}
			};
			ExecuteTransportSpans(TransportInPin, BlockSizeFrames, TransportHandler);
		}

		void Reset(const IOperator::FResetParams& InParams)
		{
			PlayOutPin->Reset();
			StopOutPin->Reset();
			*StartTimeOutPin = FTime();

			BlockSizeFrames = InParams.OperatorSettings.GetNumFramesPerBlock();
			SampleRate = InParams.OperatorSettings.GetSampleRate();

			bPlaying = false;
		}

		//void Reset(const FResetParams& ResetParams)
		//{
		//	PlayOutPin->Reset();

		//	FMidiPlayCursor::Reset(true);

		//	SetMessageFilter(FMidiPlayCursor::EFilterPassFlags::None);
		//	MidiClockInPin->RegisterHiResPlayCursor(this);

		//	CurrentTimestamp = *TimestampInPin;

		//	TriggerTick = 0;
		//	CalculateTriggerTick();
		//}

		void CalculateTriggerTick()
		{
			const FSongMaps& SongMaps = MidiClockInPin->GetSongMaps();
			TriggerTick = SongMaps.CalculateMidiTick(CurrentTimestamp, Quantize);
		}

	private:

		// Inputs
		HarmonixMetasound::FMusicTransportEventStreamReadRef TransportInPin;
		HarmonixMetasound::FMidiClockReadRef MidiClockInPin;

		FMusicTimestampReadRef TimestampInPin;

		// Outputs
		FTriggerWriteRef PlayOutPin;
		FTriggerWriteRef StopOutPin;
		FTimeWriteRef StartTimeOutPin;

		int32 BlockSizeFrames;
		float SampleRate;

		bool bPlaying = false;


		//** DATA (current state)
		FMusicTimestamp CurrentTimestamp{ 1, 1.0f };
		EMidiClockSubdivisionQuantization Quantize = EMidiClockSubdivisionQuantization::None;

		int32 TriggerTick = 0;




	};



	class FTimeStampedTransportWavePlayerControllerNode : public FNodeFacade
	{
	public:
		FTimeStampedTransportWavePlayerControllerNode(const FNodeInitData& InitData)
			: FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FTimeStampedTransportWavePlayerControllerOperator>())
		{
		}
	};

	METASOUND_REGISTER_NODE(FTimeStampedTransportWavePlayerControllerNode);
}

#undef LOCTEXT_NAMESPACE
