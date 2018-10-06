using UnrealBuildTool;
using System.IO;


public abstract class MaxLiveLinkPluginTargetBase 
	: TargetRules
{
	public abstract string Get3dsMaxVersion();
	public abstract string Get3dsMaxBasePath();

	public MaxLiveLinkPluginTargetBase(TargetInfo Target) : 
		base(Target)
	{
		Type = TargetType.Program;
        LinkType = TargetLinkType.Monolithic;

        bCompileLeanAndMeanUE = true;

        bBuildEditor = false;

        bCompileAgainstEngine = false;
        bCompileAgainstCoreUObject = true;

        bBuildWithEditorOnlyData = false;

        bIncludeADO = true;

		bCompileICU = false;

        bShouldCompileAsDLL = true;
        string OutputName = "MaxLiveLinkPlugin" + this.Get3dsMaxVersion();
        if (Target.Configuration != UnrealTargetConfiguration.Development)
        {
            OutputName = string.Format("{0}-{1}-{2}", OutputName, Target.Platform, Target.Configuration);
        }
        PostBuildSteps.Add(string.Format(
			"copy /Y \"$(EngineDir)\\Binaries\\Win64\\{0}.dll\" \"$(EngineDir)\\Binaries\\Win64\\{0}.dlu\" >nul: & echo Copied output to $(EngineDir)\\Binaries\\Win64\\{0}.dlu", 
			OutputName
		));
        PostBuildSteps.Add(string.Format(
			"copy /Y \"$(EngineDir)\\Binaries\\Win64\\{0}.*\" \"{1}\\plugins\\\" >nul: & echo Copied output to $(EngineDir)\\Binaries\\Win64\\{0}.dlu", 
			OutputName, this.Get3dsMaxBasePath()
		));
    }
}

public class MaxLiveLinkPlugin2016Target
	: MaxLiveLinkPluginTargetBase
{

	public MaxLiveLinkPlugin2016Target(TargetInfo Target) 
		: base(Target)
	{
		LaunchModuleName = 
			"MaxLiveLinkPlugin" + this.Get3dsMaxVersion()
			;
	}

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

	
}
