
#include "ILiveLinkExtendPreviewPlugin.h"

#include "IPluginManager.h"
#include "ModuleManager.h"


FString InPluginContent( const FString& RelativePath, const ANSICHAR* Extension )
{
	static FString ContentDir = IPluginManager::Get().FindPlugin( TEXT( "LiveLinkExtendPreviewPlugin" ) )->GetContentDir();
	return ( ContentDir / RelativePath ) + Extension;
}

#define LOCTEXT_NAMESPACE "LiveLinkExtendPreviewPlugin"

	


	void FLiveLinkExtendPreviewPlugin::StartupModule()
	{
	}

	bool FLiveLinkExtendPreviewPlugin::SupportsDynamicReloading()
	{
		return false;
	}


	IMPLEMENT_MODULE( FLiveLinkExtendPreviewPlugin, LiveLinkExtendPreviewPlugin );

#undef LOCTEXT_NAMESPACE