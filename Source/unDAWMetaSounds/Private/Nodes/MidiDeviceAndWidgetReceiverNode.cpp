// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeInterface.h"
#include "MetasoundParamHelper.h"
#include "MetasoundSampleCounter.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundVertex.h"

#include "HarmonixMetasound/DataTypes/MidiStream.h"
#include "HarmonixMetasound/DataTypes/MusicTransport.h"
#include "MidiStreamTrackIsolatorNode.h"
#include "MidiDeviceManager.h"

#include "MidiDeviceAndWidgetReceiverNode.h"
#include "MidiTrackIsolator.h"

#define LOCTEXT_NAMESPACE "unDAWMetasounds_MidiDeviceAndWidgetReceiverNode"

namespace unDAWMetasounds::MidiDeviceAndWidgetReceiverNode
{
	using namespace Metasound;
	using namespace HarmonixMetasound;

	const FNodeClassName& GetClassName()
	{
		static FNodeClassName ClassName
		{
			"unDAW",
			"MidiStreamInput",
			""
		};
		return ClassName;
	}

	int32 GetCurrentMajorVersion()
	{
		return 1;
	}

	namespace Inputs
	{
		DEFINE_INPUT_METASOUND_PARAM(Enable, "Enable", "Enable");
		DEFINE_INPUT_METASOUND_PARAM(MidiStream, "MidiStream", "MidiStream");
		DEFINE_INPUT_METASOUND_PARAM(MinTrackIndex, "Track Index", "Track");
		DEFINE_INPUT_METASOUND_PARAM(MaxTrackIndex, "Channel Index", "Channel");
		//DEFINE_INPUT_METASOUND_PARAM(IncludeConductorTrack, "Include Conductor Track", "Enable to include the conductor track (AKA track 0)");
	}

	namespace Outputs
	{
		DEFINE_OUTPUT_METASOUND_PARAM(MidiStream, "MidiStream", "MidiStream");
	}

	class FMidiDeviceAndWidgetReceiverOperator final : public TExecutableOperator<FMidiDeviceAndWidgetReceiverOperator>
	{
	public:
		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto InitNodeInfo = []() -> FNodeClassMetadata
				{
					FNodeClassMetadata Info;
					Info.ClassName = GetClassName();
					Info.MajorVersion = 1;
					Info.MinorVersion = 0;
					Info.DisplayName = INVTEXT("MIDI Stream Input Node");
					Info.Description = INVTEXT("merge midi inputs into an existing midi stream");
					Info.Author = PluginAuthor;
					Info.PromptIfMissing = PluginNodeMissingPrompt;
					Info.DefaultInterface = GetVertexInterface();
					Info.CategoryHierarchy = { INVTEXT("unDAW"), NodeCategories::Music };
					return Info;
				};

			static const FNodeClassMetadata Info = InitNodeInfo();

			return Info;
		}

		static const FVertexInterface& GetVertexInterface()
		{
			static const FVertexInterface Interface(
				FInputVertexInterface(
					TInputDataVertex<bool>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Enable), true),
					TInputDataVertex<FMidiStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiStream)),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MinTrackIndex), 0),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MaxTrackIndex), 0)
					//TInputDataVertex<bool>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::IncludeConductorTrack), false)
				),
				FOutputVertexInterface(
					TOutputDataVertex<FMidiStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::MidiStream))
				)
			);

			return Interface;
		}

		struct FInputs
		{
			FBoolReadRef Enabled;
			FMidiStreamReadRef MidiStream;
			FInt32ReadRef MinTrackIndex;
			FInt32ReadRef MaxTrackIndex;
			//FBoolReadRef IncludeConductorTrack;
		};

		struct FOutputs
		{
			FMidiStreamWriteRef MidiStream;
		};

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			const FInputVertexInterfaceData& InputData = InParams.InputData;

			FInputs Inputs
			{
				InputData.GetOrCreateDefaultDataReadReference<bool>(Inputs::EnableName, InParams.OperatorSettings),
				InputData.GetOrConstructDataReadReference<FMidiStream>(Inputs::MidiStreamName),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MinTrackIndexName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MaxTrackIndexName, InParams.OperatorSettings),
				//InputData.GetOrCreateDefaultDataReadReference<bool>(Inputs::IncludeConductorTrackName, InParams.OperatorSettings)
			};

			FOutputs Outputs
			{
				FMidiStreamWriteRef::CreateNew()
			};

			return MakeUnique<FMidiDeviceAndWidgetReceiverOperator>(InParams, MoveTemp(Inputs), MoveTemp(Outputs));
		}

		FMidiDeviceAndWidgetReceiverOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs, FOutputs&& InOutputs)
			: Inputs(MoveTemp(InInputs))
			, Outputs(MoveTemp(InOutputs))
		{
			Reset(InParams);
		}

		virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Inputs::EnableName, Inputs.Enabled);
			InVertexData.BindReadVertex(Inputs::MidiStreamName, Inputs.MidiStream);
			InVertexData.BindReadVertex(Inputs::MinTrackIndexName, Inputs.MinTrackIndex);
			InVertexData.BindReadVertex(Inputs::MaxTrackIndexName, Inputs.MaxTrackIndex);
			//InVertexData.BindReadVertex(Inputs::IncludeConductorTrackName, Inputs.IncludeConductorTrack);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Outputs::MidiStreamName, Outputs.MidiStream);
		}

		void Reset(const FResetParams&)
		{
			OnMidiInputDeviceChanged(TEXT("Impulse"));
		}


		void OnMidiInputDeviceChanged(FString NewSelection)
		{
			//int32 DeviceID;
			TArray<FMIDIDeviceInfo> InputDevices, OutputDevices;
			UMIDIDeviceManager::FindAllMIDIDeviceInfo(InputDevices, OutputDevices);

			//Find input device with 'Impulse' in its name
			FMIDIDeviceInfo* MyImpulseDevice = InputDevices.FindByPredicate([NewSelection](const FMIDIDeviceInfo& DeviceInfo)
				{
					return DeviceInfo.DeviceName.Contains(NewSelection);
				});

			//UMIDIDeviceManager::GetDevice
			//print name and device ID
			if (MyImpulseDevice != nullptr)
			{
				UE_LOG(LogTemp, Log, TEXT("MIDI Device Name: %s, Device ID: %d"), *MyImpulseDevice->DeviceName, MyImpulseDevice->DeviceID);
				MidiDeviceController = UMIDIDeviceManager::CreateMIDIDeviceInputController(MyImpulseDevice->DeviceID, 512);


				if (!IsValid(MidiDeviceController)) return;

				RawEventDelegateHandle = MidiDeviceController->OnMIDIRawEvent.AddRaw(this, &FMidiDeviceAndWidgetReceiverOperator::OnReceiveRawMidiMessage);
				TickOffset = Inputs.MidiStream->GetClock()->GetCurrentMidiTick();
			}
			else {
				UE_LOG(LogTemp, Log, TEXT("MIDI Device not found"));
			}


		}

		void OnReceiveRawMidiMessage(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Type, int32 Channel, int32 MessageData1, int32 MessageData2)
		{
			UE_LOG(LogTemp, Warning, TEXT("Midi Raw Event: %d %d %d %d %d"), Timestamp, Type, Channel, MessageData1, MessageData2);
			const EMIDIEventType MIDIEventType = static_cast<EMIDIEventType>(Type);

			switch (MIDIEventType)
			{
			case EMIDIEventType::NoteOn:
				UE_LOG(LogTemp, Warning, TEXT("Note On"));
				//FMidiMsg NewNoteOn = ;
				PendingMessages.Add(TTuple<int32, FMidiMsg>(Timestamp, FMidiMsg::CreateNoteOn(Channel, MessageData1, MessageData2)));
				break;
			case EMIDIEventType::NoteOff:
				UE_LOG(LogTemp, Warning, TEXT("Note Off"));
				//FMidiMsg NewMidiMsg = ;
				PendingMessages.Add(TTuple<int32, FMidiMsg>(Timestamp, FMidiMsg::CreateNoteOff(Channel, MessageData1)));
				break;
			case EMIDIEventType::ControlChange:
				UE_LOG(LogTemp, Warning, TEXT("Control Change"));
				break;
			case EMIDIEventType::ProgramChange:
				UE_LOG(LogTemp, Warning, TEXT("Program Change"));
				break;
			case EMIDIEventType::PitchBend:
				UE_LOG(LogTemp, Warning, TEXT("Pitch Bend"));
				break;
			case EMIDIEventType::NoteAfterTouch:
				UE_LOG(LogTemp, Warning, TEXT("Aftertouch"));
				break;
			default:
				UE_LOG(LogTemp, Warning, TEXT("Unknown Event"));
				break;
			}
			//construct new midi message from data
			//we'll probably need a switch here... I hope not
			//FMidiMsg NewMidiMsg(Type, MessageData1, MessageData2);
			//PendingMessages.Add(TTuple<int32, FMidiMsg>(Timestamp, NewMidiMsg));

		}

		//destructor
		virtual ~FMidiDeviceAndWidgetReceiverOperator()
		{
			UE_LOG(LogTemp, Log, TEXT("MidiDeviceAndWidgetReceiverOperator Destructor"));
			if (MidiDeviceController != nullptr)
			{
				MidiDeviceController->OnMIDIRawEvent.Remove(RawEventDelegateHandle);
				UMIDIDeviceManager::ShutDownAllMIDIDevices();
				//MidiDeviceController->
				//UMIDIDeviceManager::MidiIn
				//MidiDeviceController->ShutdownDevice();
				MidiDeviceController = nullptr;
			}
		}


		void Execute()
		{
			//Filter.SetFilterValues(*Inputs.MinTrackIndex, *Inputs.MaxTrackIndex, false);

			Outputs.MidiStream->PrepareBlock();

			if (*Inputs.Enabled)
			{
				//stream current tick?
				//int32 CurrentTick = Inputs.MidiStream->GetClock()->GetCurrentMidiTick();
				//Inputs.MidiStream->Add
				
				MergeOp.Process(*Inputs.MidiStream, PendingMessages, *Outputs.MidiStream);
				PendingMessages.Empty();
				//Filter.Process(*Inputs.MidiStream, *Outputs.MidiStream);
			}
		}
	private:
		FInputs Inputs;
		FOutputs Outputs;
		UMIDIDeviceInputController* MidiDeviceController = nullptr;
		FDelegateHandle RawEventDelegateHandle;
		int32 TickOffset = 0; //in theory we can start when the stream tick is different from the device tick, we'll see
		TArray<TTuple<int32, FMidiMsg>> PendingMessages;
		unDAWMetasounds::MidiStreamEventTrackMergeOp::FMidiStreamEventTrackMerge MergeOp;
		//unDAWMetasounds::TrackIsolatorOP::FMidiTrackIsolator Filter;
	};

	class FunDAWMidiInputNode final : public FNodeFacade
	{
	public:
		explicit FunDAWMidiInputNode(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<FMidiDeviceAndWidgetReceiverOperator>())
		{}
		virtual ~FunDAWMidiInputNode() override = default;
	};

	METASOUND_REGISTER_NODE(FunDAWMidiInputNode)
}

#undef LOCTEXT_NAMESPACE // "HarmonixMetaSound"