// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "IAudioProxyInitializer.h"
#include "MetasoundDataTypeRegistrationMacro.h"

#include "Misc/Guid.h"

#include "MetasoundPrimitives.h"
#include "MetasoundTrigger.h"
#include "MetasoundAudioBuffer.h"
#include "MetasoundWave.h"

#define WRAP_TOKEN(token)  #token
#define DECLARE_BK_PARAM(DisplayName, Description, LayerID, DataType, Init) { \
    INVTEXT(DisplayName), INVTEXT(Description), DataType, {( WRAP_TOKEN(LayerID) ), Init}},



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
            const FInput GeneratedInputs[8] =
            {

                DECLARE_BK_PARAM("unDAW Instrument Sustain Pedal State","boolean representing the current state of the sustain pedal for the SFZ instrument performer controlling this metasound",
                    unDAW Instrument.PedalState,
                    Metasound::GetMetasoundDataTypeName<bool>(), false)

            };

            const FOutput GeneratedOutputs[8] =
            {

                DECLARE_BK_PARAM("unDAW Instrument Sustain Pedal State","boolean representing the current state of the sustain pedal for the SFZ instrument performer controlling this metasound",
                    unDAW Instrument.PedalState,
                    Metasound::GetMetasoundDataTypeName<bool>(), false)

            };


#undef DECLARE_BK_PARAM_INPUT
#undef  WRAP_TOKEN       

        };

    }

