// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public abstract class MaxLiveLinkPluginTargetBase : TargetRules
{
	public MaxLiveLinkPluginTargetBase(TargetInfo Target) : base(Target)
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
        string OutputName = "MaxLiveLinkPlugin2016";
        if (Target.Configuration != UnrealTargetConfiguration.Development)
        {
            OutputName = string.Format("{0}-{1}-{2}", OutputName, Target.Platform, Target.Configuration);
        }
        PostBuildSteps.Add(string.Format("copy /Y \"$(EngineDir)\\Binaries\\Win64\\{0}.dll\" \"$(EngineDir)\\Binaries\\Win64\\{0}.dlu\" >nul: & echo Copied output to $(EngineDir)\\Binaries\\Win64\\{0}.dlu", OutputName));
        PostBuildSteps.Add(string.Format("copy /Y \"$(EngineDir)\\Binaries\\Win64\\{0}.*\" \"C:\\Program Files\\Autodesk\\3ds Max 2016\\plugins\\\" >nul: & echo Copied output to $(EngineDir)\\Binaries\\Win64\\{0}.dlu", OutputName));
    }
}

public class MaxLiveLinkPlugin2016Target : MaxLiveLinkPluginTargetBase
{
	public MaxLiveLinkPlugin2016Target(TargetInfo Target) : base(Target)
	{
		LaunchModuleName = "MaxLiveLinkPlugin2016";
	}
}