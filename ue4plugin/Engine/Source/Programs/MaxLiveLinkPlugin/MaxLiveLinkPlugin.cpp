
/*! @file		MaxLiveLinkPlugin.cpp
	@brief		3DSMaxでのLiveLinkを行うプラグインのエントリー
*/



#include	<CommonInclude.h>


#include	"MaxLiveLink.h"


IMPLEMENT_APPLICATION( MaxLiveLinkPlugin, "MaxLiveLinkPlugin");



HINSTANCE g_hInst = NULL;

#define LIVELINK_CLASS_ID	Class_ID(0x36d125ab, 0x6d2c806e)


FMaxLiveLink* liveLinkInstance_ = nullptr;

class LiveLinkClassDesc :
	public ClassDesc 
{
public:
	virtual int 			IsPublic()	{ return TRUE; }
	virtual void*			Create( BOOL loading = FALSE )
	{
		return liveLinkInstance_;
	}
	const TCHAR *	ClassName()		{ return L"MaxLiveLink"; }
	SClass_ID		SuperClassID()	{ return UTILITY_CLASS_ID; }
	Class_ID		ClassID()		{ return LIVELINK_CLASS_ID; }
	const TCHAR* 	Category()		{ return L"MaxLiveLink"; }
	void			ResetClassParams( BOOL fileReset )
	{
	}
};


static LiveLinkClassDesc LiveLinkDesc;
ClassDesc* GetLiveLinkDesc()
{
	return &LiveLinkDesc;
}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	static BOOL controlsInit = FALSE;
	switch( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		{
			MaxSDK::Util::UseLanguagePackLocale();
			g_hInst = hModule;

			if( !liveLinkInstance_ )
			{
				liveLinkInstance_ = new FMaxLiveLink( g_hInst );
			}

			DisableThreadLibraryCalls( hModule );
		}break;

	case DLL_THREAD_ATTACH:
		{

		}break;
	case DLL_THREAD_DETACH:
		{
		}break;
	}
	return( TRUE );
}



#ifdef __cplusplus  
extern "C" {
#endif 

	__declspec( dllexport ) int LibNumberClasses()
	{
		return 1;
	}


	__declspec( dllexport ) ClassDesc* LibClassDesc( int i )
	{
		switch( i ) {
		case 0: return GetLiveLinkDesc();
		default: return 0;
		}
	}


	__declspec( dllexport ) const TCHAR* LibDescription()
	{
		static const TCHAR* libDescription = L"Max Live Link";
		return libDescription;
	}


	__declspec( dllexport ) ULONG LibVersion()
	{
		return VERSION_3DSMAX;
	}

#ifdef __cplusplus  
}
#endif


