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


#undef DECLARE_BK_PARAM_NOINIT
#undef DECLARE_BK_PARAM
#undef DECLARE_BK_PARAM_OUT
#undef  WRAP_TOKEN       

        };

    }

