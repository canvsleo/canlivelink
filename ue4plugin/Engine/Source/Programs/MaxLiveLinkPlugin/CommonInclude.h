
/*! @file		CommonInclude.h
	@brief		汎用的に使用するinclude群
*/


#pragma once

#if !defined( __CommonInclude__ )
	#define __CommonInclude__

	#include "RequiredProgramMainCPPInclude.h"

	#include "Modules/ModuleManager.h"

	#include "LiveLinkProvider.h"
	#include "LiveLinkRefSkeleton.h"
	#include "LiveLinkTypes.h"
	#include "Misc/OutputDevice.h"

	

	#include "Windows/WindowsSystemIncludes.h"


	/*------- ↓ 3dsmax include ↓ ------- {{{ */

		#define TRUE 1
		#define FALSE 0

		#include "notify.h"

		#include "maxscript/maxscript.h"
		#include "maxheap.h"
		#include "export.h"
		#include "meshadj.h"
		#include "maxscript/maxwrapper/mxsobjects.h"
		#include "ISceneEventManager.h"
		#include "maxscript/macros/define_instantiation_functions.h"

		#include "max.h"
		#include "utilapi.h"
		#include "decomp.h"
		#include "modstack.h"
		#include "iskin.h"
		#include "Tab.h"
		#include "Animatable.h"
		#include "MeshNormalSpec.h"
		#include "IGame/IGameModifier.h"

		using NodeArray = TArray<INode*>;

		#undef DWORD

	/*------- ↑ 3dsmax include ↑ ------- }}} */


	DEFINE_LOG_CATEGORY_STATIC( LogMaxPlugin, Log, All );

#endif // __CommonInclude__
