// Copyright Amir BK, based on original Engine  Epic Games, Inc. All Rights Reserved.

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
#include "Misc/EngineVersionComparison.h"
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

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 4
	class FTimeStampedTransportWavePlayerControllerOperator : public TExecutableOperator<FTimeStampedTransportWavePlayerControllerOperator>, public HarmonixMetasound::FMusicTransportControllable , public FMidiPlayCursor
#else
	class FTimeStampedTransportWavePlayerControllerOperator : public TExecutableOperator<FTimeStampedTransportWavePlayerControllerOperator>, public HarmonixMetasound::FMusicTransportControllable
#endif
	{
	public:
		FTimeStampedTransportWavePlayerControllerOperator(const FOperatorSettings& InSettings,
			const FBuildOperatorParams& InParams,
			const HarmonixMetasound::FMusicTransportEventStreamReadRef& InTransport,
			const HarmonixMetasound::FMidiClockReadRef& InMidiClock,
			const FMusicTimestampReadRef& InTimestamp) :
			HarmonixMetasound::FMusicTransportControllable(HarmonixMetasound::EMusicPlayerTransportState::Prepared),
			TransportInPin(InTransport),
			MidiClockInPin(InMidiClock),
			TimestampInPin(InTimestamp),
			PlayOutPin(FTriggerWriteRef::CreateNew(InSettings)),
			StopOutPin(FTriggerWriteRef::CreateNew(InSettings)),
			StartTimeOutPin(FTimeWriteRef::CreateNew()),
			BlockSizeFrames(InSettings.GetNumFramesPerBlock()),
			SampleRate(InSettings.GetSampleRate())
		{
			Reset(InParams);
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
					Info.ClassName = { TEXT("unDAW"), TEXT("TimeStampedTransportWavePlayerController"), TEXT("") };
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

			return MakeUnique<FTimeStampedTransportWavePlayerControllerOperator>(InParams.OperatorSettings, InParams, InTransport, InMidiClock, InTimestamp);
		}

		void Execute()
		{
			using namespace HarmonixMetasound;
			// advance the outputs
			PlayOutPin->AdvanceBlock();
			StopOutPin->AdvanceBlock();

			// first let's see if our configuration has changed at all...
			if (CurrentTimestamp != *TimestampInPin)
			{
				CurrentTimestamp = *TimestampInPin;

				CalculateAndInvalidateTriggerTick();
			}
			bool isEnabled = true;
			bool AfterStartTimeStamp = false;
			int32 timeStampBlockFrameIndex = -1;

			if (isEnabled)
			{
				for (const FTickSpan& Span : TickSpans)
				{
					if (Span.FromTick < TriggerTick && Span.ThruTick >= TriggerTick)
					{
						timeStampBlockFrameIndex = Span.BlockFrameIndex;
						//this means we hit the timestamp, we need to check if we're before it or after it
						UE_LOG(LogTemp, Log, TEXT("TimeStamp Hit? blockFrameIndex %d"), timeStampBlockFrameIndex)
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 4
							if (CurrentTick < Span.BlockFrameIndex)
							{
								AfterStartTimeStamp = false;
								timeStampBlockFrameIndex = -1;
							}
							else {
								AfterStartTimeStamp = true;
							}
#endif

						break;
					}
				}
			}
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 4
			if (CurrentTick < TriggerTick && bPlaying == true)
			{
				//bPlaying = false;
				timeStampBlockFrameIndex = -1;
			}
#endif

			TickSpans.Empty(8);

			//UE_LOG(LogTemp, Log, TEXT("trigger tick %d"), TriggerTick)

			TransportSpanProcessor TransportHandler = [this, timeStampBlockFrameIndex](int32 StartFrameIndex, int32 EndFrameIndex, EMusicPlayerTransportState CurrentState)
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
						if (timeStampBlockFrameIndex >= 0)
						{
							PlayOutPin->TriggerFrame(timeStampBlockFrameIndex);
							bPlaying = true;
							return EMusicPlayerTransportState::Playing;
						}
						return EMusicPlayerTransportState::Starting;

					case EMusicPlayerTransportState::Playing:
						return EMusicPlayerTransportState::Playing;

					case EMusicPlayerTransportState::Seeking:

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 4
						if (ReceivedSeekWhileStopped())
						{
							
							// Assumes the MidiClock is stopped for the remainder of the block.
							*StartTimeOutPin = FTime(MidiClockInPin->GetCurrentHiResMs() * 0.001f);
							if (CurrentTick < TriggerTick) return EMusicPlayerTransportState::Pausing;
						}
						else
						{
							StopOutPin->TriggerFrame(StartFrameIndex);
							int32 PlayFrameIndex = FMath::Min(StartFrameIndex + 1, EndFrameIndex);
							if (timeStampBlockFrameIndex >= 0 || CurrentTick >= TriggerTick)
							{
								const auto startTickTime = MidiClockInPin->GetSongMaps().TickToMs(TriggerTick);
								float seekTarget = (MidiClockInPin->GetCurrentHiResMs() - startTickTime) * 0.001f - (BlockSizeFrames - PlayFrameIndex) / SampleRate;
								float unmoddedSeek = (MidiClockInPin->GetCurrentHiResMs()) * 0.001f - (BlockSizeFrames - PlayFrameIndex) / SampleRate;
								*StartTimeOutPin = FTime(seekTarget);
								UE_LOG(LogTemp, Log, TEXT("We enter this block, the tick now is %d the tick to seek is %d, offset time %f, original seek %f"), CurrentTick, TriggerTick, seekTarget, unmoddedSeek)
									PlayOutPin->TriggerFrame(PlayFrameIndex);
							}
							else {
								*StartTimeOutPin = FTime(MidiClockInPin->GetCurrentHiResMs() * 0.001f);
							}
							// Assumes the MidiClock is playing for the remainder of the block.
						}
						// Here we will return that we want to be in the same state we were in before this request to
						// seek since we can seek "instantaneously"...
#endif
						return GetTransportState();

					case EMusicPlayerTransportState::Continuing:
						// Assumes the StartTimeOutPin won't change for the remainder of the block.
						PlayOutPin->TriggerFrame(StartFrameIndex);
						bPlaying = true;
						return EMusicPlayerTransportState::Playing;

					case EMusicPlayerTransportState::Pausing:
						bPlaying = false;
						StopOutPin->TriggerFrame(StartFrameIndex);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 4
						// Assumes the MidiClock is paused for the remainder of the block.
						*StartTimeOutPin = FTime(MidiClockInPin->GetCurrentHiResMs() * 0.001f);
#endif
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
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 4
			FMidiPlayCursor::Reset(true);

			SetMessageFilter(FMidiPlayCursor::EFilterPassFlags::None);
			MidiClockInPin->RegisterHiResPlayCursor(this);
#endif

			CurrentTimestamp = *TimestampInPin;

			BlockSizeFrames = InParams.OperatorSettings.GetNumFramesPerBlock();
			SampleRate = InParams.OperatorSettings.GetSampleRate();

			bPlaying = false;

			TriggerTick = 0;
			CalculateAndInvalidateTriggerTick();

			//UE_LOG(LogTemp,Log, TEXT("Do we get reset?"))
		}

		void CalculateAndInvalidateTriggerTick()
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 4
			const FSongMaps& SongMaps = MidiClockInPin->GetSongMaps();
			TriggerTick = SongMaps.CalculateMidiTick(CurrentTimestamp, Quantize);
#endif
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

		//** DATA (current state)
		FMusicTimestamp CurrentTimestamp{ 1, 1.0f };
		EMidiClockSubdivisionQuantization Quantize = EMidiClockSubdivisionQuantization::None;

		int32 TriggerTick = 0;
		//Harmonix changed the API for FMidiPlayCursor in 5.5, so we need to check the engine version
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 4
		//** BEGIN FMidiPlayCursor
		virtual void SeekToTick(int32 Tick) override { SeekThruTick(Tick - 1); }
		virtual void SeekThruTick(int32 Tick) override {
			int32 TickProceedingThisAdvance = CurrentTick;
			FMidiPlayCursor::SeekThruTick(Tick);

			// don't trigger if seeking backward or no progress being made...
			if (Tick < TickProceedingThisAdvance || TickProceedingThisAdvance == CurrentTick)
			{
				return;
			}

			TickSpans.Emplace(TickProceedingThisAdvance, CurrentTick, MidiClockInPin->GetCurrentBlockFrameIndex(), true);
		}

		virtual void AdvanceThruTick(int32 Tick, bool IsPreRoll) override {
			int32 TickProceedingThisAdvance = CurrentTick;
			FMidiPlayCursor::AdvanceThruTick(Tick, IsPreRoll);

			// don't trigger during preroll or if no progress being made...
			if (IsPreRoll || TickProceedingThisAdvance == CurrentTick)
			{
				return;
			}
			//UE_LOG(LogTemp, Log, TEXT("we get into the midi cursor events? %d"), TriggerTick)
			TickSpans.Emplace(TickProceedingThisAdvance, CurrentTick, MidiClockInPin->GetCurrentBlockFrameIndex(), false);
		}
		// We have to override this to disambiguate the FMidiPlayCursor Reset and the MS operator Reset
	//	virtual void Reset(bool ForceNoBroadcast) override { FMidiPlayCursor::Reset(ForceNoBroadcast); }
		//** END FMidiPlayCursor

#endif
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