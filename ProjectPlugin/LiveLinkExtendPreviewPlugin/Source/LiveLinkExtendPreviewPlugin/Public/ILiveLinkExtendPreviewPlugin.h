
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "LiveLinkExtendPreviewPlugin"

class FLiveLinkExtendPreviewPlugin : public IModuleInterface
{
public:

	virtual void	StartupModule() override;
	virtual bool	SupportsDynamicReloading() override;

};

#undef LOCTEXT_NAMESPACE