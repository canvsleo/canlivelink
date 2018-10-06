using UnrealBuildTool;
using System.Collections.Generic;

public class MaxLiveLinkPlugin2019Target
	: MaxLiveLinkPluginTargetBase
{
	public override string Get3dsMaxVersion()
	{
		return "2019";
	}
	public override string Get3dsMaxBasePath()
	{
		return string.Format(
			"C:\\Program Files\\Autodesk\\3ds Max {0}",
			this.Get3dsMaxVersion()
		);
	}

	public MaxLiveLinkPlugin2019Target(TargetInfo Target) 
		: base(Target)
	{
		LaunchModuleName = 
			"MaxLiveLinkPlugin" + this.Get3dsMaxVersion()
			;
	}
}
