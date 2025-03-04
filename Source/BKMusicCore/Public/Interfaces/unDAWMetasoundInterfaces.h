// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "IAudioProxyInitializer.h"
#include "MetasoundDataTypeRegistrationMacro.h"

#include "Misc/Guid.h"

#include "MetasoundPrimitives.h"
#include "MetasoundTrigger.h"
#include "MetasoundAudioBuffer.h"
#include "HarmonixMetasound/DataTypes/MidiStream.h"
#include "HarmonixMetasound/DataTypes/MidiClock.h"
#include "HarmonixMetasound/DataTypes/MusicTimeStamp.h"
#include "HarmonixMetasound/DataTypes/MidiAsset.h"
#include "MetasoundDataReference.h"
#include "MetasoundWave.h"



namespace unDAW::Metasounds

{


	class BKMUSICCORE_API FunDAWInstrumentRendererInterface : public Audio::FParameterInterface
	{
		inline static Audio::FParameterInterfacePtr InstancePointer = nullptr;

	public:
		FunDAWInstrumentRendererInterface() : FParameterInterface("unDAW Instrument Renderer", { 0, 1 })
		{
			Inputs.Append(GeneratedInputs);
			Outputs.Append(GeneratedOutputs);
		}

		static Audio::FParameterInterfacePtr GetInterface()
		{
			if (!InstancePointer.IsValid())
			{
				InstancePointer = MakeShared<FunDAWInstrumentRendererInterface>();
			}
			return InstancePointer;
		}

		static void RegisterInterface()
		{
			//UE_LOG(FK_SFZ_Logs, Display, TEXT("Registering unDAW SFZ Parameter Interfaces"));
			Audio::IAudioParameterInterfaceRegistry& InterfaceRegistry = Audio::IAudioParameterInterfaceRegistry::Get();
			InterfaceRegistry.RegisterInterface(GetInterface());
		}

		~FunDAWInstrumentRendererInterface() {};

	private:

		//so this is how we wind up declaring params, at least I don't have to do it 40 times
		const FInput GeneratedInputs[1] =
		{
			{ INVTEXT("MidiStream"), INVTEXT("Midi Stream to rendered with this instrument"), Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiStream>(),{ FName("unDAW Instrument.MidiStream") } },
		};

		const FOutput GeneratedOutputs[2] =
		{
			{ INVTEXT("Audio Out L"), INVTEXT("Instrument Audio Output"), Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>(),{ ("unDAW Instrument.Audio L") } },
			{ INVTEXT("Audio Out R"), INVTEXT("Instrument Audio Output"), Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>(),{ ("unDAW Instrument.Audio R") } },
		};
	};

	class BKMUSICCORE_API FunDAWMidiInsertInterface : public Audio::FParameterInterface
	{
		inline static Audio::FParameterInterfacePtr InstancePointer = nullptr;

	public:
		FunDAWMidiInsertInterface() : FParameterInterface("unDAW Midi Insert", { 0, 1 })
		{
			Inputs.Append(GeneratedInputs);
			Outputs.Append(GeneratedOutputs);
		}

		static Audio::FParameterInterfacePtr GetInterface()
		{
			if (!InstancePointer.IsValid())
			{
				InstancePointer = MakeShared<FunDAWMidiInsertInterface>();
			}
			return InstancePointer;
		}

		static void RegisterInterface()
		{
			//UE_LOG(FK_SFZ_Logs, Display, TEXT("Registering unDAW SFZ Parameter Interfaces"));
			Audio::IAudioParameterInterfaceRegistry& InterfaceRegistry = Audio::IAudioParameterInterfaceRegistry::Get();
			InterfaceRegistry.RegisterInterface(GetInterface());
		}

		~FunDAWMidiInsertInterface() {};

	private:

		//so this is how we wind up declaring params, at least I don't have to do it 40 times
		const FInput GeneratedInputs[1] =
		{
			{ INVTEXT("MidiStream"), INVTEXT("Midi Stream to be processed by this insert"), Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiStream>(),{ FName("unDAW Instrument.MidiStream") } },
		};

		const FOutput GeneratedOutputs[1] =
		{
			{ INVTEXT("MidiStream"), INVTEXT("Midi Stream output after processing"), Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiStream>(),{ FName("unDAW Instrument.MidiStream") } },
		};
	};

	//graph action patches are patches that can be inserted into the from the game thread, their main purpose is to perform some logic on the audio thread
	//and fire back a trigger that will be passed back to the registring actor when they are performed
	class BKMUSICCORE_API FunDAWMusicalActionInterface : public Audio::FParameterInterface
	{
		inline static Audio::FParameterInterfacePtr InstancePointer = nullptr;

	public:
		FunDAWMusicalActionInterface() : FParameterInterface("TimeStamped Musical Action", { 0, 1 })
		{
			Inputs.Append(GeneratedInputs);
			Outputs.Append(GeneratedOutputs);
		}

		static Audio::FParameterInterfacePtr GetInterface()
		{
			if (!InstancePointer.IsValid())
			{
				InstancePointer = MakeShared<FunDAWMusicalActionInterface>();
			}
			return InstancePointer;
		}

		static void RegisterInterface()
		{
			//UE_LOG(FK_SFZ_Logs, Display, TEXT("Registering unDAW SFZ Parameter Interfaces"));
			Audio::IAudioParameterInterfaceRegistry& InterfaceRegistry = Audio::IAudioParameterInterfaceRegistry::Get();
			InterfaceRegistry.RegisterInterface(GetInterface());
		}

		~FunDAWMusicalActionInterface() {};

	private:

		//so this is how we wind up declaring params, at least I don't have to do it 40 times
		const FInput GeneratedInputs[1] =
		{
			{ INVTEXT("PerformAction"), INVTEXT("Trigger Action to be performed"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW Instrument.MidiStream") } },
		};

		const FOutput GeneratedOutputs[2] =
		{
			{ INVTEXT("OnActionPerformed"), INVTEXT("Trigger output when action is performed, is sent back to the calling actor"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW Action.OnPerformed") } },
			{ INVTEXT("OnActionComplete"), INVTEXT("Trigger output when the action is complete and should be cleaned up"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW Action.OnComplete") } },
		};
	};


	class BKMUSICCORE_API FunDAWAudibleActionInterface : public Audio::FParameterInterface
	{

		inline static Audio::FParameterInterfacePtr InstancePointer = nullptr;

	public:

		//static const FInput MidiClockIn = 


		FunDAWAudibleActionInterface() : FParameterInterface("Audible Action", { 0, 1 })

		{
			Inputs.Add({ INVTEXT("MidiClock"), INVTEXT("Midi Clock Input"), Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiClock>(),{ FName("unDAW.Midi Clock") } });
			Inputs.Add({ INVTEXT("Transport"), INVTEXT("Transport Input"), Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMusicTransportEventStream>(),{ FName("unDAW.Transport") } });
			Inputs.Add({ INVTEXT("MidiAsset"), INVTEXT("Midi Asset Input"), Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiAsset>(),{ FName("unDAW.Midi Asset") } });
			Inputs.Add({ INVTEXT("OnAdded"), INVTEXT("Trigger input when the action is added to the graph"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW Action.OnAdded") } });
			Inputs.Add({ INVTEXT("TimeStampBar"), INVTEXT("Bar to start the action at"), Metasound::GetMetasoundDataTypeName<int>(),{ FName("unDAW.Action.TimeStampBar") } });
			Inputs.Add({ INVTEXT("TimeStampBeat"), INVTEXT("Beat to start the action at"), Metasound::GetMetasoundDataTypeName<float>(),{ FName("unDAW.Action.TimeStampBeat") } });
			Outputs.Append(AudioOutputs);
			Outputs.Add({ INVTEXT("OnActionComplete"), INVTEXT("Trigger output when the action is complete and should be cleaned up"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW Action.OnComplete") } });
			Outputs.Add({ INVTEXT("MidiStream"), INVTEXT("If the action also includes a midi player this can be used to watch its output"), Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiStream>(),{ FName("unDAW.Midi Stream") } });

		}

		const FOutput AudioOutputs[2] =
		{
			{ INVTEXT("Audio Out L"), INVTEXT("Audible Action Audio Output L"), Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>(),{ ("unDAW Insert.Audio L") } },
			{ INVTEXT("Audio Out R"), INVTEXT("Audible Action Audio Output R"), Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>(),{ ("unDAW Insert.Audio R") } },
		};

		static Audio::FParameterInterfacePtr GetInterface()
		{
			if (!InstancePointer.IsValid())
			{
				InstancePointer = MakeShared<FunDAWAudibleActionInterface>();
			}
			return InstancePointer;
		}

		static void RegisterInterface()
		{
			//UE_LOG(FK_SFZ_Logs, Display, TEXT("Registering unDAW SFZ Parameter Interfaces"));
			Audio::IAudioParameterInterfaceRegistry& InterfaceRegistry = Audio::IAudioParameterInterfaceRegistry::Get();
			InterfaceRegistry.RegisterInterface(GetInterface());
		}



	};


	class BKMUSICCORE_API FunDAWCustomInsertInterface : public Audio::FParameterInterface
	{
		inline static Audio::FParameterInterfacePtr InstancePointer = nullptr;

	public:
		FunDAWCustomInsertInterface() : FParameterInterface("unDAW Custom Insert", { 0, 1 })
		{
			Inputs.Append(GeneratedInputs);
			Outputs.Append(GeneratedOutputs);
		}

		static Audio::FParameterInterfacePtr GetInterface()
		{
			if (!InstancePointer.IsValid())
			{
				InstancePointer = MakeShared<FunDAWCustomInsertInterface>();
			}
			return InstancePointer;
		}

		static void RegisterInterface()
		{
			//UE_LOG(FK_SFZ_Logs, Display, TEXT("Registering unDAW SFZ Parameter Interfaces"));
			Audio::IAudioParameterInterfaceRegistry& InterfaceRegistry = Audio::IAudioParameterInterfaceRegistry::Get();
			InterfaceRegistry.RegisterInterface(GetInterface());
		}

		~FunDAWCustomInsertInterface() {};

	private:

		//so this is how we wind up declaring params, at least I don't have to do it 40 times
		const FInput GeneratedInputs[2] =
		{
			{ INVTEXT("Audio In L"), INVTEXT("Insert audio input L"), Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>(),{ FName("unDAW Insert.Audio In L") } },
			{ INVTEXT("Audio In R"), INVTEXT("Insert audio input R"), Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>(),{ FName("unDAW Insert.Audio In R") } },
		};

		const FOutput GeneratedOutputs[2] =
		{
			{ INVTEXT("Audio Out L"), INVTEXT("Insert Audio Output"), Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>(),{ ("unDAW Insert.Audio L") } },
			{ INVTEXT("Audio Out R"), INVTEXT("Insert Audio Output"), Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>(),{ ("unDAW Insert.Audio R") } },
		};
	};

	class BKMUSICCORE_API FunDAWMasterGraphInterface : public Audio::FParameterInterface
	{
		inline static Audio::FParameterInterfacePtr InstancePointer = nullptr;

	public:

		// This determines the name of the interface as shown in the MetaSounds graphs and when interacted with by the builder system,
		// the versioning is actually important as the graphs are picky about inserting nodes and interfaces with mismatching versions
		// This interface has both inputs and outputs so we append both arrays, but in case an interface only has one just don't append the array.
		FunDAWMasterGraphInterface() : FParameterInterface("unDAW Session Renderer", { 0, 1 })
		{
			Inputs.Append(GeneratedInputs);
			Outputs.Append(GeneratedOutputs);
		}

		// Boiler plate really, I'm actually not 100% sure what this does, I think some of the epic interfaces are templated and you can create different variants
		// For my purposes there's no real value in templating the interfaces so they're each a singleton, the only purpose of this function is allow registering this interface to the engine
		// The interface is being registered in the Module .cpp file in the 'InitModule' method.
		static Audio::FParameterInterfacePtr GetInterface()
		{
			if (!InstancePointer.IsValid())
			{
				InstancePointer = MakeShared<FunDAWMasterGraphInterface>();
			}
			return InstancePointer;
		}

		//this is the method that gets called by the module init method.
		static void RegisterInterface()
		{
			Audio::IAudioParameterInterfaceRegistry& InterfaceRegistry = Audio::IAudioParameterInterfaceRegistry::Get();
			InterfaceRegistry.RegisterInterface(GetInterface());
		}

		~FunDAWMasterGraphInterface() {};

	private:

		//so adding I/Os is essentially done here by creating const arrays which are append when the interface is constructed,
		// I think this method is not so bad given that interfaces are not really mutable anyway, there are more options that can be given to each
		// I/O, I only use a few, it's important to observe the namespaces of the data types, the harmonix data types are in a new name space.
		// DO NOTE: the declared size of the array must match the number of elements, otherwise the code won't compile.

		// Declaring too many elements in the array will throw an exception when starting the editor or game.
		const FInput GeneratedInputs[11] =
		{
			{ INVTEXT("Play"), INVTEXT("Play Trigger"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW.Transport.Play") } },
			{ INVTEXT("Prepare"), INVTEXT("Prepare Trigger"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW.Transport.Prepare") }},
			{ INVTEXT("Pause"), INVTEXT("Pause Trigger"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW.Transport.Pause") }},
			{ INVTEXT("Stop"), INVTEXT("Stop Trigger"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW.Transport.Stop") } },
			{ INVTEXT("Kill"), INVTEXT("Kill Trigger"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW.Transport.Kill") } },
			{ INVTEXT("Seek"), INVTEXT("Seek Trigger"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW.Transport.Seek") } },
			{ INVTEXT("TimeStamp Seek"), INVTEXT("Timestamp Seek Trigger"), Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(),{ FName("unDAW.Transport.SeekTimeStamp") } },
			{ INVTEXT("Seek Target"), INVTEXT("Time into the MIDI Clock to seek"), Metasound::GetMetasoundDataTypeName<float>(),{ ("unDAW.Transport.SeekTarget"), 0.0f } },
			{ INVTEXT("Midi Playrate"), INVTEXT("Playrate of MIDI clock"), Metasound::GetMetasoundDataTypeName<float>(),{ ("unDAW.Midi.Speed"), 1.0f } },
			{ INVTEXT("TimeStamp Seek Target"), INVTEXT("Timestamp into the MIDI Clock to seek"), Metasound::GetMetasoundDataTypeName<FMusicTimestamp>(),{ FName("unDAW.Transport.SeekTimeStampTarget") } },
			{ INVTEXT("Playrate affects pitch"), INVTEXT("Should MIDI pitch be affected by playrate"), Metasound::GetMetasoundDataTypeName<bool>(),{ FName("unDAW.Midi.PlayratePitch") } },
		};

		//same trick for outputs
		const FOutput GeneratedOutputs[2] =
		{
			{INVTEXT("MidiClock"), INVTEXT("Midi Clock Output"),  Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiClock>(), {FName(TEXT("unDAW.Midi Clock"))}},
			 {INVTEXT("MidiStream"), INVTEXT("Midi Stream Output"),  Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiStream>(), {FName(TEXT("unDAW.Midi Stream"))}}
		};
	};
}

#undef DECLARE_BK_PARAM_NOINIT
#undef DECLARE_BK_PARAM
#undef DECLARE_BK_PARAM_OUT
#undef  WRAP_TOKEN
