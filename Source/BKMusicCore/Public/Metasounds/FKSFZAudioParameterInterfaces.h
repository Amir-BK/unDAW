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
#include "UnDAWSFZAsset.h"


class F_FK_SFZ_Asset_Proxy;
class FFK_SFZ_Region_Performance_Proxy;

namespace Metasound
{
    // this class is a proxy for an entire instrument that can be accessed as a Data Type in Metasounds 
    class BKMUSICCORE_API F_FK_SFZ_Instrument_Asset
    {
    public:


        F_FK_SFZ_Instrument_Asset() = default;
        F_FK_SFZ_Instrument_Asset(const F_FK_SFZ_Instrument_Asset&) = default;
        F_FK_SFZ_Instrument_Asset& operator=(const F_FK_SFZ_Instrument_Asset& Other) = default;
        F_FK_SFZ_Instrument_Asset(const TSharedPtr<Audio::IProxyData>& InInitData);

        bool IsInitialized() const { return SFZInstrumentProxy.IsValid(); }
        const F_FK_SFZ_Asset_Proxy* GetProxy() const { return SFZInstrumentProxy.Get(); }
    private:
        TSharedPtr<const F_FK_SFZ_Asset_Proxy> SFZInstrumentProxy;

    };

    DECLARE_METASOUND_DATA_REFERENCE_TYPES(F_FK_SFZ_Instrument_Asset, BKMUSICCORE_API, FK_SFZ_InstrumentAssetTypeInfo, FK_SFZ_InstrumentAssetReadRef, F_FK_SFZ_InstrumentWriteRef)

    class BKMUSICCORE_API F_FK_SFZ_Region_Data
    {
    public:

        F_FK_SFZ_Region_Data() = default;
        F_FK_SFZ_Region_Data(const F_FK_SFZ_Region_Data&) = default;
        F_FK_SFZ_Region_Data& operator=(const F_FK_SFZ_Region_Data& Other) = default;
        F_FK_SFZ_Region_Data(const TSharedPtr<Audio::IProxyData>& InInitData);

        bool IsInitialized() const { return SFZRegionProxy.IsValid(); }
        const FFK_SFZ_Region_Performance_Proxy* GetProxy() const { return SFZRegionProxy.Get(); }
    private:
        TSharedPtr<const FFK_SFZ_Region_Performance_Proxy> SFZRegionProxy;

    };

    DECLARE_METASOUND_DATA_REFERENCE_TYPES(F_FK_SFZ_Region_Data, BKMUSICCORE_API, F_FK_SFZ_Region_DataTypeInfo, F_FK_SFZ_Region_DataReadRef, F_FK_SFZ_Region_WriteRef)

}

namespace FK_SFZ::Metasounds
{
    class BKMUSICCORE_API FFKSFZAudioParameterInterfaces : public Audio::FParameterInterface
    {
        inline static Audio::FParameterInterfacePtr singletonPointer = nullptr;
        
    public:
        FFKSFZAudioParameterInterfaces() : FParameterInterface("unDAW SFZ", {0, 1})
        {
           Inputs.Append(GeneratedInputs);
        }


         static Audio::FParameterInterfacePtr GetInterface()
         {
             if (!singletonPointer.IsValid())
             {
                 singletonPointer = MakeShared<FFKSFZAudioParameterInterfaces>();
             }

             return singletonPointer;
         }

         static void RegisterInterface()
         {
             UE_LOG(FK_SFZ_Logs, Display, TEXT("Registering unDAW SFZ Parameter Interfaces"));
             Audio::IAudioParameterInterfaceRegistry& InterfaceRegistry = Audio::IAudioParameterInterfaceRegistry::Get();
             InterfaceRegistry.RegisterInterface(GetInterface());
         }

         ~FFKSFZAudioParameterInterfaces() {};

    private:

#define WRAP_TOKEN(token)  #token
#define DECLARE_SFZ_PARAM_INPUT(DisplayName, Description, LayerID, DataType, Init) { \
    INVTEXT(DisplayName), INVTEXT(Description), DataType, {( WRAP_TOKEN(LayerID) ), Init}},

        //so this is how we wind up declaring params, at least I don't have to do it 40 times 
        const FInput GeneratedInputs[8] =
        {

            DECLARE_SFZ_PARAM_INPUT("BK SFZ Sustain Pedal State","boolean representing the current state of the sustain pedal for the SFZ instrument performer controlling this metasound",
                BK SFZ.Internals.PedalState,
                Metasound::GetMetasoundDataTypeName<bool>(), false)
            DECLARE_SFZ_PARAM_INPUT("BK SFZ Hard Note Off","Trigger that sends hard note off that is used to kill trigger the release state of the note even when sustain is engaged",
                BK SFZ.Internals.HardNoteOff,
                Metasound::GetMetasoundDataTypeName<Metasound::FTrigger>(), false)
            DECLARE_SFZ_PARAM_INPUT("BK SFZ Layer 0 Data","Wraps all data for a certain region layer",
                BK SFZ.Layer_0_Region,
                Metasound::GetMetasoundDataTypeName<Metasound::F_FK_SFZ_Region_Data>(), static_cast<UObject*>(nullptr))
            DECLARE_SFZ_PARAM_INPUT("BK SFZ Layer 1 Data","Wraps all data for a certain region layer",
                BK SFZ.Layer_1_Region,
                Metasound::GetMetasoundDataTypeName<Metasound::F_FK_SFZ_Region_Data>(), static_cast<UObject*>(nullptr))
            DECLARE_SFZ_PARAM_INPUT("BK SFZ Layer 2 Data", "Wraps all data for a certain region layer",
                                    BK SFZ.Layer_2_Region,
                Metasound::GetMetasoundDataTypeName<Metasound::F_FK_SFZ_Region_Data>(), static_cast<UObject*>(nullptr))
            DECLARE_SFZ_PARAM_INPUT("BK SFZ Layer 3 Data", "Wraps all data for a certain region layer",
                                    BK SFZ.Layer_3_Region, Metasound::GetMetasoundDataTypeName<Metasound::F_FK_SFZ_Region_Data>(), 
                                    static_cast<UObject*>(nullptr))
            DECLARE_SFZ_PARAM_INPUT("BK SFZ Release Layer 0 Data", "Release layers are played on note off",
                                   BK SFZ.Relase_0_Region,
               Metasound::GetMetasoundDataTypeName<Metasound::F_FK_SFZ_Region_Data>(), static_cast<UObject*>(nullptr))
            DECLARE_SFZ_PARAM_INPUT("BK SFZ Release Layer 1 Data", "Release layers are played on note off",
                                   BK SFZ.Release_1_Region,
               Metasound::GetMetasoundDataTypeName<Metasound::F_FK_SFZ_Region_Data>(), static_cast<UObject*>(nullptr))

            };
#undef DECLARE_SFZ_PARAM_INPUT
#undef  WRAP_TOKEN       

    };

}


