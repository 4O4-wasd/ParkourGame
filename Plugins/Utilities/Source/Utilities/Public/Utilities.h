
#pragma once

#include "Modules/ModuleManager.h"

class FUtilitiesModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
