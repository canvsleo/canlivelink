// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public abstract class MaxLiveLinkPluginBase : ModuleRules
{
    public MaxLiveLinkPluginBase(ReadOnlyTargetRules Target) : base(Target)
    {

        bUseRTTI = true;

        PublicDefinitions.AddRange( new string[] {
            "_UNICODE",
            "MULTI_PROCESSING=0",
            //"PCL_ONLY_CORE_POINT_TYPES",
            //"PCL_NO_PRECOMPILE",
            //"UE_ENGINE_DIRECTORY=\"D:/project/UnrealEngine-release/Engine\""
        } );



        // For LaunchEngineLoop.cpp include.  You shouldn't need to add anything else to this line.
        PrivateIncludePaths.AddRange( new string[] {
            "Runtime/Launch/Public", "Runtime/Launch/Private"
        });

        // Unreal dependency modules
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "ApplicationCore",
            "Projects",
            "UdpMessaging",
            "LiveLinkInterface",
            "LiveLinkMessageBusFramework",
        });
        /*
		PrivateDependencyModuleNames.AddRange( new string[] 
		{
			"Core",
            "CoreUObject",
            "AudioMixer",
            "ApplicationCore",
            "AutomationController",
            "AutomationWorker",
            "DerivedDataCache",
            "SessionServices",
            "MediaUtils",
            "HeadMountedDisplay",
            "MRMesh",
            "SlateRHIRenderer",
            "SlateNullRenderer",
            "MoviePlayer",
            "TaskGraph",
            "ProfilerService",
            "Projects",
            "UdpMessaging",
            "LiveLinkInterface",
            "LiveLinkMessageBusFramework",
		} );
		*/


        {
			string MaxVersionString = GetMaxVersion();
			string MaxInstallFolder = @"C:\Program Files\Autodesk\3ds Max " + MaxVersionString + @" SDK\maxsdk";

			if( Directory.Exists( MaxInstallFolder ) )
			{

				PublicDefinitions.Add("NT_PLUGIN=1");
                PublicDefinitions.Add("REQUIRE_IOSTREAM=1");

				//PrivateIncludePaths.Add(Path.Combine(MaxInstallFolder, "include"));

				if (Target.Platform == UnrealTargetPlatform.Win64)  // @todo: Support other platforms?
				{
					PublicLibraryPaths.Add(Path.Combine(MaxInstallFolder, @"lib\x64\Release"));
					PrivateIncludePaths.Add(Path.Combine(MaxInstallFolder, @"include"));

					// Maya libraries we're depending on
					PublicAdditionalLibraries.AddRange(new string[]
						{
							"shlwapi.lib",
							"assetmanagement.lib",
							"msimg32.lib",
							"comctl32.lib",
							"version.lib",
							"wininet.lib",
							"mnmath.lib",
							"maxscrpt.lib",
							"poly.lib",
							"bmm.lib",
							"core.lib",
							"expr.lib",
							"geom.lib",
							"gfx.lib",
							"maxutil.lib",
							"mesh.lib",
							"winmm.lib"
						}
					);
				}
			}
		}
	}
	
	public abstract string GetMaxVersion();
}

public class MaxLiveLinkPlugin2016 : MaxLiveLinkPluginBase
{
	public MaxLiveLinkPlugin2016(ReadOnlyTargetRules Target) : base(Target)
	{
	}
	
	public override string GetMaxVersion() { return "2016"; }
}
