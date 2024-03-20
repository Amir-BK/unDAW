// Copyright Amir Ben-Kiki and unDAW org.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"






class unDAWMetaSoundsModule : public IModuleInterface
{
public:
	UPROPERTY(BlueprintReadOnly)
	FString PluginContentDir;




	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
