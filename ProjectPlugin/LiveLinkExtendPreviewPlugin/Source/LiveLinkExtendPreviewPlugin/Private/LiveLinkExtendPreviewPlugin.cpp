
#include "IPluginManager.h"

#include "Editor.h"

#include "ModuleManager.h"
#include "CoreDelegates.h"

#include "Features/IModularFeatures.h"
#include "LevelEditor.h"

#include "LiveLinkClient.h"

/**
* Implements the Messaging module.
*/

#define LOCTEXT_NAMESPACE "LiveLinkExtendPreviewPlugin"

	FString InPluginContent( const FString& RelativePath, const ANSICHAR* Extension )
	{
		static FString ContentDir = IPluginManager::Get().FindPlugin( TEXT( "LiveLinkExtendPreviewPlugin" ) )->GetContentDir();
		return ( ContentDir / RelativePath ) + Extension;
	}

	class LiveLinkExtendPreviewPlugin : public IModuleInterface
	{
	public:
		virtual void StartupModule() override
		{
		}

		virtual void ShutdownModule() override
		{
		}

		virtual bool SupportsDynamicReloading() override
		{
			return false;
		}

	private:
	};

	IMPLEMENT_MODULE( LiveLinkExtendPreviewPlugin, LiveLinkExtendPreviewPlugin );

#undef LOCTEXT_NAMESPACE