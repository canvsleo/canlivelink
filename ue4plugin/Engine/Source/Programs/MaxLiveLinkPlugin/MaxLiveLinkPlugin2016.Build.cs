using UnrealBuildTool;
using System.IO;
public abstract class MaxLiveLinkPluginBase 
	: ModuleRules
{
    public MaxLiveLinkPluginBase(ReadOnlyTargetRules Target) : base(Target)
    {

        bUseRTTI = true;

        PublicDefinitions.AddRange( new string[] {
            "_UNICODE",
            "MULTI_PROCESSING=0",
        } );

        PrivateIncludePaths.AddRange( new string[] {
            "Runtime/Launch/Public",
			"Runtime/Launch/Private"
        });

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

        {
			string MaxVersionString = this.Get3dsMaxVersion();
			string MaxInstallFolder = this.Get3dsMaxBasePath() + @" SDK\maxsdk";

			if( Directory.Exists( MaxInstallFolder ) )
			{

				PublicDefinitions.Add("NT_PLUGIN=1");
                PublicDefinitions.Add("REQUIRE_IOSTREAM=1");

				if (Target.Platform == UnrealTargetPlatform.Win64)
				{
					PublicLibraryPaths.Add(Path.Combine(MaxInstallFolder, @"lib\x64\Release"));
					PrivateIncludePaths.Add(Path.Combine(MaxInstallFolder, @"include"));

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
	
	public abstract string Get3dsMaxVersion();
	public abstract string Get3dsMaxBasePath();
}

public class MaxLiveLinkPlugin2016 
	: MaxLiveLinkPluginBase
{
	public override string Get3dsMaxVersion()
	{
		return "2016";
	}
	public override string Get3dsMaxBasePath()
	{
		return string.Format(
			"C:\\Program Files\\Autodesk\\3ds Max {0}",
			this.Get3dsMaxVersion()
		);
	}

	public MaxLiveLinkPlugin2016(ReadOnlyTargetRules Target) 
		: base(Target)
	{
	}
}
