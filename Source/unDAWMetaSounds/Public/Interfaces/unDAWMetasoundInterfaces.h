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
#include "MetasoundWave.h"

#define WRAP_TOKEN(token)  #token
#define DECLARE_BK_PARAM(DisplayName, Description, LayerID, DataType, Init) { \
    INVTEXT(DisplayName), INVTEXT(Description), DataType, {( WRAP_TOKEN(LayerID)), Init}},

#define DECLARE_BK_PARAM_NOINIT(DisplayName, Description, LayerID, DataType) { \
    INVTEXT(DisplayName), INVTEXT(Description), DataType, {FName(WRAP_TOKEN(LayerID)) }},

#define DECLARE_BK_PARAM_OUT(DisplayName, Description, LayerID, DataType) { \
    INVTEXT(DisplayName), INVTEXT(Description), DataType, {( WRAP_TOKEN(LayerID) )}},


    namespace unDAW::Metasounds
    {
        class UNDAWMETASOUNDS_API FunDAWInstrumentRendererInterface : public Audio::FParameterInterface
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

                DECLARE_BK_PARAM_NOINIT("MidiStream","Midi Stream to rendered with this instrument",
                    unDAW Instrument.MidiStream,
                    Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiStream>())



            };

            const FOutput GeneratedOutputs[2] =
            {

                DECLARE_BK_PARAM_OUT("Audio Out L","Instrument Audio Output",
                    unDAW Instrument.Audio L,
                    Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>())
             DECLARE_BK_PARAM_OUT("Audio Out R","Instrument Audio Output",
                    unDAW Instrument.Audio R,
                    Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>())

            };



        };

        class UNDAWMETASOUNDS_API FunDAWMidiInsertInterface : public Audio::FParameterInterface
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

                DECLARE_BK_PARAM_NOINIT("MidiStream","Midi Stream to rendered with this instrument",
                    unDAW Instrument.MidiStream,
                    Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiStream>())


            };

            const FOutput GeneratedOutputs[1] =
            {

                DECLARE_BK_PARAM_NOINIT("MidiStream","Midi Stream to rendered with this instrument",
                    unDAW Instrument.MidiStream,
                    Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiStream>())


            };



        };


        class UNDAWMETASOUNDS_API FunDAWCustomInsertInterface : public Audio::FParameterInterface
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

                DECLARE_BK_PARAM_NOINIT("Audio In L","Insert audio input L",
                    unDAW Insert.Audio In L,
                    Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>())
            DECLARE_BK_PARAM_NOINIT("Audio In R","Insert audio input R",
                    unDAW Insert.Audio In R,
                    Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>())

    

            };

            const FOutput GeneratedOutputs[2] =
            {

                DECLARE_BK_PARAM_OUT("Audio Out L","Insert Audio Output",
                    unDAW Insert.Audio L,
                    Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>())
                DECLARE_BK_PARAM_OUT("Audio Out R","Insert Audio Output",
                    unDAW Insert.Audio R,
                    Metasound::GetMetasoundDataTypeName<Metasound::FAudioBuffer>())

            };



        };

        class UNDAWMETASOUNDS_API FunDAWMasterGraphInterface : public Audio::FParameterInterface
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
