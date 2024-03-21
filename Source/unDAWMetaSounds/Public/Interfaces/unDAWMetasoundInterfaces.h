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
            inline static Audio::FParameterInterfacePtr singletonPointer = nullptr;

        public:
            FunDAWInstrumentRendererInterface() : FParameterInterface("unDAW Instrument Renderer", { 0, 1 })
            {
                Inputs.Append(GeneratedInputs);
                Outputs.Append(GeneratedOutputs);
            }

            static Audio::FParameterInterfacePtr GetInterface()
            {
                if (!singletonPointer.IsValid())
                {
                    singletonPointer = MakeShared<FunDAWInstrumentRendererInterface>();
                }
                return singletonPointer;
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
            const FInput GeneratedInputs[2] =
            {

                DECLARE_BK_PARAM_NOINIT("MidiStream","Midi Stream to rendered with this instrument",
                    unDAW Instrument.MidiStream,
                    Metasound::GetMetasoundDataTypeName<HarmonixMetasound::FMidiStream>())

                DECLARE_BK_PARAM("Track","Midi Track To Render With This Instrument",
                    unDAW Instrument.MidiTrack,
                    Metasound::GetMetasoundDataTypeName<int>(), 0)

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


        class UNDAWMETASOUNDS_API FunDAWCustomInsertInterface : public Audio::FParameterInterface
        {
            inline static Audio::FParameterInterfacePtr singletonPointer = nullptr;

        public:
            FunDAWCustomInsertInterface() : FParameterInterface("unDAW Custom Insert", { 0, 1 })
            {
                Inputs.Append(GeneratedInputs);
                Outputs.Append(GeneratedOutputs);
            }

            static Audio::FParameterInterfacePtr GetInterface()
            {
                if (!singletonPointer.IsValid())
                {
                    singletonPointer = MakeShared<FunDAWCustomInsertInterface>();
                }
                return singletonPointer;
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
            inline static Audio::FParameterInterfacePtr singletonPointer = nullptr;

        public:
            FunDAWMasterGraphInterface() : FParameterInterface("unDAW Session Renderer", { 0, 1 })
            {
                Inputs.Append(GeneratedInputs);
                //Outputs.Append(GeneratedOutputs);
            }

            static Audio::FParameterInterfacePtr GetInterface()
            {
                if (!singletonPointer.IsValid())
                {
                    singletonPointer = MakeShared<FunDAWMasterGraphInterface>();
                }
                return singletonPointer;
            }

            static void RegisterInterface()
            {
                //UE_LOG(FK_SFZ_Logs, Display, TEXT("Registering unDAW SFZ Parameter Interfaces"));
                Audio::IAudioParameterInterfaceRegistry& InterfaceRegistry = Audio::IAudioParameterInterfaceRegistry::Get();
                InterfaceRegistry.RegisterInterface(GetInterface());
            }

            ~FunDAWMasterGraphInterface() {};

        private:


            //so this is how we wind up declaring params, at least I don't have to do it 40 times 
            const FInput GeneratedInputs[9] =
            {

                DECLARE_BK_PARAM_NOINIT("Play","Play Trigger",
                    unDAW.Transport.Play,
                    Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>())
                 DECLARE_BK_PARAM_NOINIT("Prepare","Prepare Trigger",
                    unDAW.Transport.Prepare,
                    Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>())
                DECLARE_BK_PARAM_NOINIT("Pause","Pause Trigger",
                    unDAW.Transport.Pause,
                    Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>())
                 DECLARE_BK_PARAM_NOINIT("Stop","Stop Trigger",
                    unDAW.Transport.Stop,
                    Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>())
                 DECLARE_BK_PARAM_NOINIT("Kill","Kill Trigger",
                    unDAW.Transport.Kill,
                    Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>())
                  DECLARE_BK_PARAM_NOINIT("Seek","Seek Trigger",
                    unDAW.Transport.Seek,
                    Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>())
                  DECLARE_BK_PARAM_NOINIT("TimeStamp Seek","Timestamp Seek Trigger",
                    unDAW.Transport.SeekTimeStamp,
                    Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>())

                DECLARE_BK_PARAM("Seek Target","Time into the MIDI Clock to seek",
                    unDAW.Transport.SeekTarget,
                    Metasound::GetMetasoundDataTypeName<float>(), 0.0f)

                 DECLARE_BK_PARAM_NOINIT("TimeStamp Seek Target","Timestamp into the MIDI Clock to seek",
                    unDAW.Transport.SeekTimeStampTarget,
                    Metasound::GetMetasoundDataTypeName<FMusicTimestamp>())

            };

            //const FOutput GeneratedOutputs[0] =
            //{


            //};



        };

    }






#undef DECLARE_BK_PARAM_NOINIT
#undef DECLARE_BK_PARAM
#undef DECLARE_BK_PARAM_OUT
#undef  WRAP_TOKEN       
