// Fill out your copyright notice in the Description page of Project Settings.


#include "Metasounds/FKSFZAudioParameterInterfaces.h"
#include "IAudioParameterInterfaceRegistry.h"
#include "MetasoundDataReference.h"
#include  "FKSFZAsset.h"


using namespace Metasound;



Metasound::F_FK_SFZ_Instrument_Asset::F_FK_SFZ_Instrument_Asset(const TSharedPtr<Audio::IProxyData>& InInitData)
{
	if (!InInitData.IsValid()) return;
	SFZInstrumentProxy = MakeShared<const F_FK_SFZ_Asset_Proxy>(InInitData->GetAs<F_FK_SFZ_Asset_Proxy>());
};
DEFINE_METASOUND_DATA_TYPE(Metasound::F_FK_SFZ_Instrument_Asset, "FlyKick SFZ Instrument")

Metasound::F_FK_SFZ_Region_Data::F_FK_SFZ_Region_Data(const TSharedPtr<Audio::IProxyData>& InInitData)
{
	if (!InInitData.IsValid()) return;
	SFZRegionProxy = MakeShared<const FFK_SFZ_Region_Performance_Proxy>(InInitData->GetAs<FFK_SFZ_Region_Performance_Proxy>());
};

DEFINE_METASOUND_DATA_TYPE(Metasound::F_FK_SFZ_Region_Data, "FlyKick SFZ Region")

Audio::FParameterInterfacePtr FK_SFZ::Metasounds::FFKSFZAudioParameterInterfaces::GetInterface()
{
	if (!singletonPointer.IsValid())
	{
		singletonPointer = MakeShared<FFKSFZAudioParameterInterfaces>();
	}

	return singletonPointer;
}

void FK_SFZ::Metasounds::FFKSFZAudioParameterInterfaces::RegisterInterface()
{
	UE_LOG(FK_SFZ_Logs, Display, TEXT("Registering Flykick SFZ Parameter Interfaces"));
	Audio::IAudioParameterInterfaceRegistry& InterfaceRegistry = Audio::IAudioParameterInterfaceRegistry::Get();
	InterfaceRegistry.RegisterInterface(GetInterface());
}

FK_SFZ::Metasounds::FFKSFZAudioParameterInterfaces::~FFKSFZAudioParameterInterfaces()
{
}
